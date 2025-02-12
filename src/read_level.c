#include "read_level.h"
#include "worldInteraction.h"
#include "settings.h"
#include "duscript.h"
#include "logging.h"

#include <stdio.h>
#include <string.h>


static inline void combine_tile_properties(Tile *dst, Tile *other)
{
    if (dst->is_exit_level == 0)
        dst->is_exit_level = other->is_exit_level;
    if (dst->is_floor == 0)
        dst->is_floor = other->is_floor;
    if (dst->is_blocker == 0)
        dst->is_blocker = other->is_blocker;
    if (dst->is_restricted == 0)
        dst->is_restricted = other->is_restricted;
    if (dst->is_clear_restriction == 0)
        dst->is_clear_restriction = other->is_clear_restriction;
    if (dst->is_exit_point == 0)
        dst->is_exit_point = other->is_exit_point;
    if (dst->is_wall == 0)
        dst->is_wall = other->is_wall;
    if (dst->is_blood_stained == 0)
        dst->is_blood_stained = other->is_blood_stained;
    if (dst->durability == 0)
        dst->durability = other->durability;
    if (dst->is_positional_trigger == 0)
        dst->is_positional_trigger = other->is_positional_trigger;
}

static inline void place_lev_object(World *world, int x, int y, int id, int room_to)
{
    Tile tile = create_tile(id);
    if (tile.valid)
    {
        if (tile.is_exit_point && tile.data == 1 && room_to == 1)
        {
            world->plr.x = x * TILESIZE + 15;
            world->plr.y = y * TILESIZE + 15;
            tile = create_tile(TILE_SYM_FLOOR);
        }
        combine_tile_properties(&world->map[room_to - 1][x][y], &tile);
        if (tile.data != 0)
        {
            world->map[room_to - 1][x][y].data = tile.data;
        }
    }
    else if (id >= 200 && id <= 205)
    {
        if (!world->rooms_visited[room_to - 1])
        {
            spawn_enemy(x, y, id - 200, room_to, world);
        }
    }
    else if (id >= 300 && id <= 306)
    {
        if (!world->rooms_visited[room_to - 1])
        {
            spawn_potion(x * TILESIZE + HALFTILESIZE, y * TILESIZE + HALFTILESIZE, id - 300, room_to, world, POTION_PRESET_RANGE_START, POTION_PRESET_RANGE_END);
        }
    }
    else
    {
        LOG("ERROR: map object id '%d' not recognized\n", id);
    }
}

static inline void level_read_new_format(World *world, int room_to, FILE *f)
{
    world->boss_fight_config = &world->boss_fight_configs[room_to - 1];
    DuScriptVariable *var;
    int file_start = ftell(f);
    DuScriptState state = du_script_init();
    int boss_exists = world->boss != NULL;

    var = du_script_variable(&state, "game_modifiers");
    sprintf(var->value, "%d", world->game_modifiers);
    var->read_only = 1;

    while (!feof(f))
    {
        char buf[DU_SCRIPT_MAX_STR_LEN];
        fgets(buf, sizeof(buf), f);
        if (feof(f))
            break;
        // level editor metadata
        if (*buf == '#')
        {
            continue;
        }
        int ret = du_script_execute_line(&state, buf);
        if (ret == 0)
        {
            continue;
        }
        else if (ret == 1)
        {
            fseek(f, file_start, SEEK_SET);
            continue;
        }
        else if (*buf == '$')
        {
            LOG_TRACE("Found $\n");
            int room = -1;
            sscanf(buf + 1, "%d", &room);
            if (room >= 1 && room <= ROOMCOUNT)
            {
                LOG_TRACE("Reading bossfight for room %d\n", room);
                read_bfconfig_new(f, &world->boss_fight_configs[room - 1], world->game_modifiers);
                world->boss_fight = 1;
            }
        }
        else if (*buf != '*') // Checking this is just defensive coding; all lines starting with * should return 0.
        {
            int id = -1, x = -1, y = -1, room = -1;
            sscanf(buf, "%d %d %d %d", &id, &x, &y, &room);
            place_lev_object(world, x, y, id, room);
            if (id == -1)
                LOG("unparsed line: %s\n", buf);
        }
    }
    fclose(f);
    var = du_script_variable(&state, "name");
    if (strlen(var->value) < 64)
    {
        strcpy(world->mission_display_name, var->value);
    }
    for (int i = 1; i <= 10; i++)
    {
        char story_var[16];
        sprintf(story_var, "story%d", i);
        var = du_script_variable(&state, story_var);
        if (*var->value && strlen(var->value) < 61)
        {
            strcpy(world->story_after_mission[i - 1], var->value);
            world->story_after_mission_lines = i;
        }
    }

    var = du_script_variable(&state, "wall_color");
    if (*var->value)
    {
        float r = 0, g = 0, b = 0;
        sscanf(var->value, "%f %f %f", &r, &g, &b);
        LOG_TRACE("Map color %f %f %f\n", r, g, b);
        world->map_wall_color[0] = r;
        world->map_wall_color[1] = g;
        world->map_wall_color[2] = b;
    }
    var = du_script_variable(&state, "no_more_levels");
    world->final_level = *var->value ? 1 : 0;
    var = du_script_variable(&state, "mute_bosstalk");
    world->play_boss_sound = *var->value ? 0 : 1;
    var = du_script_variable(&state, "story_image");
    if (*var->value)
    {
        get_data_filename(world->custom_story_image, var->value);
    }

    if (world->boss && !boss_exists)
    {
        world->boss->rate = world->boss_fight_config->fire_rate;
        world->boss->health = world->boss_fight_config->health;
    }

    world->rooms_visited[room_to - 1] = 1;
}

int read_level(World *world, int mission, int room_to)
{
    char buf[256];

    int room_from = world->current_room;

    memset(world->map, 0, sizeof(world->map));

    // Default colors
    if (mission % 3 + 1 == 1)
    {
        world->map_wall_color[0] = 2.0f / 3;
        world->map_wall_color[1] = 0;
        world->map_wall_color[2] = 0;
    }
    else if (mission % 3 + 1 == 2)
    {
        world->map_wall_color[0] = 1 / 8.0f;
        world->map_wall_color[1] = 1 / 2.0f;
        world->map_wall_color[2] = 4.0f / 5;
    }
    else
    {
        world->map_wall_color[0] = 2 / 5.0f;
        world->map_wall_color[1] = 1 / 2.0f;
        world->map_wall_color[2] = 2.0f / 5;
    }

    char mission_name[256];
    sprintf(mission_name, DATADIR "%s\\mission%d", get_game_settings()->mission_pack, mission);

    world->boss_fight = 0;
    sprintf(world->mission_display_name, "Level %d", mission);
    world->story_after_mission_lines = 0;
    world->custom_story_image[0] = 0;

    char special_filename[256];
    sprintf(special_filename, "%s-mode-%d", mission_name, world->game_modifiers);
    FILE *f = fopen(special_filename, "r");
    if (f == NULL)
    {
        f = fopen(mission_name, "r");
        if (f == NULL)
            return -1;
    }

    fgets(buf, 256, f); // version
    if (buf[0] != 'X')
    {
        printf("ERROR: legacy format file!\n");
        fclose(f);
        return -1;
    }
    level_read_new_format(world, room_to, f);
    return 0;
}