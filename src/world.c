#include "world.h"
#include "predictableRandom.h"
#include "logging.h"
#include "vfx.h"
#include "settings.h"
#include <math.h>
#include <stdio.h>

static void init_spritesheet(World *world)
{
  if (!get_game_settings()->custom_resources)
  {
    world->spr = al_load_bitmap(DATADIR "sprites.png");
  }
  else
  {
    char path[256];
    sprintf(path, DATADIR "/%s/sprites.png", get_game_settings()->mission_pack);
    world->spr = al_load_bitmap(path);
  }
  al_convert_mask_to_alpha(world->spr, al_map_rgb(255, 0, 255));
}

void init_world(World *world)
{
    init_spritesheet(world);

    world->kills = 0;
    world->play_boss_sound = 1;

    memset(world->floor_shade_map, 0, sizeof(world->floor_shade_map));
    for (int i = 0; i < ROOMCOUNT; i++)
    {
        world->rooms_visited[i] = 0;
    }
    clear_visual_fx(world);
    for (int i = 0; i < BULLETCOUNT; i++)
    {
        world->bullets[i].owner = NULL;
    }
    for (int i = 0; i < ENEMYCOUNT; i++)
    {
        Enemy *enm = &world->enm[i];
        enm->alive = enm->killed = 0;
        enm->dx = 1;
        enm->dy = 0;
        enm->reload = 10;
        enm->shots = 1;
        enm->ammo = -1;
        enm->anim = rand() % 15;
        enm->death_animation = 999;
        memset(enm->bodyparts, 0, sizeof(enm->bodyparts));
    }
    world->boss = NULL;

    world->potion_duration = 0;
    world->potion_effect_flags = 0;
    memset(world->potions, 0, sizeof(world->potions));
    world->potion_turbo_mode = 0;
    world->potion_healing_counter = 0;
    world->potion_shield_counter = 0;
    memset(world->visual_fx.flames, 0, sizeof(world->visual_fx.flames));

    world->boss_fight_config = world->boss_fight_configs;
    world->current_room = 1;
}

Tile create_tile(int symbol)
{
    const int sym_restrictions_start = 500;
    const int sym_restriction_clears_start = 510;
    const int max_restrictions = 10;

    const int sym_positional_triggers_start = 600;
    const int sym_positional_triggers_count = 10;

    const int sym_exit_points_start = TILE_SYM_EXIT_POINT(0);

    Tile t;
    memset(&t, 0, sizeof(t));
    int tile_properties_set = 0;
    if (symbol == TILE_SYM_EXIT_LEVEL)
    {
        t.is_exit_level = 1;
        t.valid = 1;
        tile_properties_set = 1;
    }
    if ((symbol == TILE_SYM_FLOOR) ||
        (symbol >= sym_restrictions_start && symbol < sym_restrictions_start + 2 * max_restrictions) ||
        (symbol >= sym_positional_triggers_start && symbol < sym_positional_triggers_start + sym_positional_triggers_count))
    {
        t.is_floor = 1;
        t.valid = 1;
        tile_properties_set = 1;
    }
    if (symbol >= sym_restriction_clears_start && symbol < sym_restriction_clears_start + max_restrictions)
    {
        t.is_clear_restriction = 1;
        t.data = symbol - sym_restriction_clears_start;
        tile_properties_set = 1;
    }
    if (symbol >= sym_positional_triggers_start && symbol < sym_positional_triggers_start + sym_positional_triggers_count)
    {
        t.is_positional_trigger = 1;
        t.data = symbol - sym_positional_triggers_start;
        tile_properties_set = 1;
    }
    if ((symbol == TILE_SYM_WALL1) || (symbol == TILE_SYM_LAVA) || (symbol == TILE_SYM_WALL2) ||
        (symbol >= sym_restrictions_start && symbol < sym_restrictions_start + max_restrictions) || symbol == TILE_SYM_BREAKABLE_WALL)
    {
        t.is_blocker = 1;
        t.valid = 1;
        tile_properties_set = 1;
    }
    if (symbol >= sym_restrictions_start && symbol < sym_restrictions_start + max_restrictions)
    {
        t.is_restricted = 1;
        t.data = symbol - sym_restrictions_start;
        tile_properties_set = 1;
    }
    if (symbol == TILE_SYM_BREAKABLE_WALL)
    {
        t.data = WALL_NORMAL;
        t.durability = 5;
        t.is_wall = 1;
        tile_properties_set = 1;
    }
    if (symbol == TILE_SYM_WALL1)
    {
        t.data = WALL_NORMAL;
        t.is_wall = 1;
        tile_properties_set = 1;
    }
    if (symbol == TILE_SYM_LAVA)
    {
        t.data = WALL_LAVA;
        t.is_wall = 1;
        tile_properties_set = 1;
    }
    if (symbol == TILE_SYM_WALL2)
    {
        t.data = WALL_PENTAGRAM;
        t.is_wall = 1;
        tile_properties_set = 1;
    }
    if (symbol > sym_exit_points_start)
    {
        t.is_exit_point = 1;
        t.valid = 1;
        t.data = symbol - sym_exit_points_start;
        tile_properties_set = 1;
    }
    if (!tile_properties_set)
    {
        t.is_floor = 1;
    }
    return t;
}

inline Tile *get_tile_at(World *world, int x, int y)
{
    return &world->map[world->current_room - 1][x / TILESIZE][y / TILESIZE];
}

inline Tile *ns_get_tile_at(World *world, int x, int y)
{
    return &world->map[world->current_room - 1][x][y];
}

inline int ns_get_wall_type_at(World *world, int x, int y)
{
    Tile *t = &(world->map[world->current_room - 1][x][y]);
    return t->is_wall ? t->data : 0;
}

inline int get_wall_type_at(World *world, int x, int y)
{
    return ns_get_wall_type_at(world, x / TILESIZE, y / TILESIZE);
}

void init_player(World *world, Enemy *plrautosave)
{
    Enemy *plr = &world->plr;
    if (!plrautosave->alive)
    {
        // this initializes the player the same way enemy 0 was initialized
        *plr = world->enm[0];
        plr->alive = 1;
        plr->killed = 0;
        plr->sprite = -1;
        plr->shots = 1;
        plr->health = 3;
        plr->rate = 7;
        if ((*world->game_modifiers & GAMEMODIFIER_BRUTAL) != 0)
        {
            plr->rate = 12;
        }
        plr->ammo = 15;
        plr->gold = 0;
        plr->hurts_monsters = 1;
    }
    else
    {
        *plr = *plrautosave;
    }
}

inline int check_potion_effect(World *w, int effect_id)
{
    return w->potion_duration > 0 && ((w->potion_effect_flags & effect_id) != 0);
}

inline int get_plr_speed(World *world)
{
    int plr_speed = 4;
    if (check_potion_effect(world, POTION_EFFECT_FAST_PLAYER))
    {
      plr_speed++;
      if (world->potion_turbo_mode)
      {
        plr_speed += 2;
      }
    }
    return plr_speed;
}