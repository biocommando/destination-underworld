#include "read_level.h"
#include "worldInteraction.h"
#include "settings.h"
#include "variables.h"
#include "logging.h"
#include "record_file.h"
#include "sha1/du_dmac.h"

#include "command_file/generated/dispatch_read_level.h"
#include "command_file/generated/dispatch_enemy_properties.h"
#include "command_file/generated/dispatch_mission_counts.h"

#include <stdio.h>
#include <string.h>

static int check_authentication(const char *file_to_check)
{
    int err = 0;

    if (get_game_settings()->require_authentication)
    {
        char fname[256];
        sprintf(fname, DATADIR "%s/auth.dat", get_game_settings()->mission_pack);
        char file_hash_hex[256] = "N/A";
        int ret = record_file_scanf(fname, file_to_check, "%*s %s", file_hash_hex);
        if (ret == 0)
            LOG_ERROR("Error reading auth.dat entry for file %s\n", file_to_check);
        char hash[DMAC_SHA1_HASH_SIZE];
        memset(hash, 0, sizeof(hash));
        convert_sha1_hex_to_hash(hash, file_hash_hex);
        dmac_sha1_set_ctx(AUTH_CTX_CONF);
        err = dmac_sha1_verify_file(file_to_check, hash);
    }

    return err;
}

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
            spawn_potion(TO_PIXEL_COORDINATES(x), TO_PIXEL_COORDINATES(y), id - 300, room_to, world, 0);
        }
    }
    else
    {
        LOG("ERROR: map object id '%d' not recognized\n", id);
    }
}

void dispatch__handle_read_level_header(struct read_level_header_DispatchDto *dto)
{
    if (dto->version == 3)
        return;
    LOG_FATAL("legacy format file!\n");
    exit(1);
}

void dispatch__handle_read_level_object(struct read_level_object_DispatchDto *dto)
{
    place_lev_object(dto->state->world, dto->x, dto->y, dto->id, dto->room);
}

void dispatch__handle_read_level_condition(struct read_level_condition_DispatchDto *dto)
{
    LOG("dispatch__handle_condition: <%s>\n", dto->expression);
    const char *p = dto->expression;
    while (p)
    {
        char logical_op[command_file_LINE_MAX] = "";
        char name[command_file_LINE_MAX] = "";
        char val[command_file_LINE_MAX] = "";
        char op = 0;
        if (sscanf(p, "%s %c %s %s", name, &op, val, logical_op) < 3)
            return;
        LOG_TRACE("Compare %s %c %s : %s\n", name, op, val, logical_op);
        const char *var = get_var(name, dto->state->variables);
        int same = var && !strcmp(var, val);
        if ((op == '=' && same) || (op == '!' && !same))
        {
            LOG_TRACE("Compare result true\n");
            if (!strcmp("and", logical_op))
            {
                p = strstr(p, " and ");
                if (p)
                {
                    p += 5;
                }
            }
            else
            {
                LOG_TRACE("condition ok -> jump to <%s>\n", logical_op);
                strcpy(dto->skip_label, dto->label);
                break;
            }
        }
        else
        {
            LOG_TRACE("Compare result false\n");
            break;
        }
    }
}

void dispatch__handle_read_level_goto(struct read_level_goto_DispatchDto *dto)
{
    strcpy(dto->skip_label, dto->label);
}

void dispatch__handle_read_level_set_var(struct read_level_set_var_DispatchDto *dto)
{
    set_var(dto->name, dto->value, dto->state->variables);
}

void dispatch__handle_read_level_script_line(struct read_level_script_line_DispatchDto *dto)
{
    if (!dto->state->bfconfig)
        return;
    int ret = read_bfconfig_line(dto->value, dto->state->bfconfig, *dto->state->world->game_modifiers, &dto->state->bfevent);
    if (ret != 0)
    {
        bfconfig_finalize(dto->state->bfconfig);
        dto->state->bfconfig = NULL;
    }
}

void dispatch__handle_read_level_script_start(struct read_level_script_start_DispatchDto *dto)
{
    int room = dto->room_number;
    if (dto->state->bfconfig)
    {
        bfconfig_finalize(dto->state->bfconfig);
        dto->state->bfconfig = NULL;
    }
    if (room >= 1 && room <= ROOMCOUNT)
    {
        LOG_TRACE("Reading bossfight for room %d\n", room);
        dto->state->bfconfig = &dto->state->world->boss_fight_configs[room - 1];
        bfconfig_init(dto->state->bfconfig);
        dto->state->world->boss_fight = 1;
    }
}

static inline void read_level_cmd_file(World *world, int room_to, const char *filename)
{ 
    int boss_exists = world->boss != NULL;
    LevelState read_level_state;
    memset(&read_level_state, 0, sizeof(LevelState));
    VarState variables = create_VarState();
    char game_mod_str[20];
    sprintf(game_mod_str, "%d", *world->game_modifiers);
    set_var("game_modifiers", game_mod_str, &variables);
    set_var_readonly("game_modifiers", &variables);

    read_level_state.variables = &variables;
    read_level_state.world = world;
    read_command_file(filename, dispatch__read_level, &read_level_state);

    const char *var = get_var("name", &variables);
    if (var && strlen(var) < 64)
    {
        strcpy(world->mission_display_name, var);
    }
    for (int i = 1; i <= 10; i++)
    {
        char story_var[16];
        sprintf(story_var, "story%d", i);
        var = get_var(story_var, &variables);
        if (var && strlen(var) < 61)
        {
            strcpy(world->story_after_mission[i - 1], var);
            world->story_after_mission_lines = i;
        }
    }

    var = get_var("wall_color", &variables);
    if (var)
    {
        float r = 0, g = 0, b = 0;
        sscanf(var, "%f %f %f", &r, &g, &b);
        LOG_TRACE("Map color %f %f %f\n", r, g, b);
        world->map_wall_color[0] = r;
        world->map_wall_color[1] = g;
        world->map_wall_color[2] = b;
    }
    var = get_var("mute_bosstalk", &variables);
    world->play_boss_sound = var ? 0 : 1;
    var = get_var("story_image", &variables);
    if (var)
    {
        get_data_filename(world->custom_story_image, var);
    }

    free_VarState(&variables);

    if (world->boss && !boss_exists)
    {
        world->boss->rate = world->boss_fight_config->fire_rate;
        world->boss->health = world->boss_fight_config->health;
    }

    world->rooms_visited[room_to - 1] = 1;
}

int read_level(World *world, int mission, int room_to)
{
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
    sprintf(mission_name, DATADIR "%s/mission%d", get_game_settings()->mission_pack, mission);

    world->boss_fight = 0;
    sprintf(world->mission_display_name, "Level %d", mission);
    world->story_after_mission_lines = 0;
    world->custom_story_image[0] = 0;

    char special_filename[280];
    sprintf(special_filename, "%s-mode-%d", mission_name, *world->game_modifiers);
    const char *filename = special_filename;
    FILE *f = fopen(filename, "r");
    int auth_check_result;
    if (f == NULL)
    {
        filename = mission_name;
        f = fopen(filename, "r");
        if (f == NULL)
            return -1;
    }
    auth_check_result = check_authentication(filename);
    fclose(f);
    if (auth_check_result)
    {
        LOG_FATAL("level authentication failed!!");
        exit(1);
    }

   read_level_cmd_file(world, room_to, filename);

    return 0;
}

void dispatch__handle_enemy_properties_config(struct enemy_properties_config_DispatchDto *dto)
{
    struct enemy_config *e = &dto->state[dto->enemy_type];
    e->turret = dto->turret;
    e->rate = dto->rate;
    e->health = dto->health;
    e->gold = dto->gold;
    e->fast = dto->fast;
    e->hurts_monsters = dto->hurts_monsters;
    e->potion_for_potion_only = dto->potion_for_potion_only;
}

void read_enemy_configs(World *world)
{
    char fname[256];
    sprintf(fname, DATADIR "%s/enemy-properties.dat", get_game_settings()->mission_pack);
    if (check_authentication(fname))
    {
        LOG_FATAL("enemy config authentication failed!!");
        exit(1);
    }
    
    read_command_file(fname, dispatch__enemy_properties, world->enemy_configs);
}

void dispatch__handle_mission_counts_mode_override(struct mission_counts_mode_override_DispatchDto *dto)
{
    if (dto->state->game_mode == dto->mode)
    {
        dto->state->count = dto->count;
    }
}

void dispatch__handle_mission_counts_initial_count(struct mission_counts_initial_count_DispatchDto *dto)
{
    if (!dto->state->override_set)
    {
        dto->state->count = dto->count;
    }
}

int read_mission_count(int game_mode)
{
    char fname[256];
    sprintf(fname, DATADIR "%s/mission-counts.dat", get_game_settings()->mission_pack);

    SetMissionCount smc;
    memset(&smc, 0, sizeof(smc));
    smc.game_mode = game_mode;

    read_command_file(fname, dispatch__mission_counts, &smc);

    return smc.count;
}
