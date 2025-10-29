#include "world.h"
#include "predictableRandom.h"
#include "logging.h"
#include "vfx.h"
#include "settings.h"
#include "game_tuning.h"
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

    world->time_stamp = 0;
    world->kills = 0;
    world->play_boss_sound = 1;

    for (int i = 0; i < ROOMCOUNT; i++)
    {
        world->rooms_visited[i] = 0;
    }
    world->visual_fx.management_list = &world->management_list;
    clear_visual_fx(&world->visual_fx, 1);
    add_managed_list(&world->bullets, &world->management_list);
    add_managed_list(&world->enm, &world->management_list);
    add_managed_list(&world->killed_enemy_stats, &world->management_list);
    world->boss = NULL;

    world->potion_duration = 0;
    world->potion_effect_flags = 0;
    add_managed_list(&world->potions, &world->management_list);
    world->potion_turbo_mode = 0;
    world->potion_healing_counter = 0;
    world->potion_shield_counter = 0;

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
        t.durability = get_tuning_params()->breakable_wall_durability;
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

inline const Tile *get_tile_at(const World *world, int x, int y)
{
    WITH_SANITIZED_TILE_COORDINATES(x, y)
    {
        return &world->map[world->current_room - 1][ok_x][ok_y];
    }
    return NULL;
}

inline const Tile *ns_get_tile_at(const World *world, int x, int y)
{
    WITH_SANITIZED_NS_TILE_COORDINATES(x, y)
    {
        return &world->map[world->current_room - 1][ok_x][ok_y];
    }
    return NULL;
}

inline Tile *get_tile_at_mut(World *world, int x, int y)
{
    WITH_SANITIZED_TILE_COORDINATES(x, y)
    {
        return &world->map[world->current_room - 1][ok_x][ok_y];
    }
    return NULL;
}

inline Tile *ns_get_tile_at_mut(World *world, int x, int y)
{
    WITH_SANITIZED_NS_TILE_COORDINATES(x, y)
    {
        return &world->map[world->current_room - 1][ok_x][ok_y];
    }
    return NULL;
}

inline int ns_get_wall_type_at(const World *world, int x, int y)
{
    WITH_SANITIZED_NS_TILE_COORDINATES(x, y)
    {
        const Tile *t = &(world->map[world->current_room - 1][ok_x][ok_y]);
        return t->is_wall ? t->data : 0;
    }
    return 0;
}

inline int get_wall_type_at(const World *world, int x, int y)
{
    WITH_SANITIZED_TILE_COORDINATES(x, y)
    {
        return ns_get_wall_type_at(world, ok_x, ok_y);
    }
    return 0;
}

void init_player(World *world, Enemy *plrautosave)
{
    const GameTuningParams *gt = get_tuning_params();
    Enemy *plr = &world->plr;
    if (!plrautosave->alive)
    {
        set_enemy_defaults(plr);
        plr->alive = 1;
        plr->killed = 0;
        plr->sprite = -1;
        plr->shots = gt->weapon_1_num_shots;
        plr->health = gt->kill_health_cap;
        plr->rate = gt->weapon_1_rate;
        if ((*world->game_modifiers & GAMEMODIFIER_BRUTAL) != 0)
        {
            plr->rate = gt->weapon_1_brutal_rate;
            plr->shots = gt->weapon_1_brutal_shots;
        }
        plr->ammo = gt->ammo_cap;
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
    const GameTuningParams *gt = get_tuning_params();
    int plr_speed = gt->plr_speed;
    if (check_potion_effect(world, POTION_EFFECT_FAST_PLAYER))
    {
      plr_speed += gt->fast_potion_plr_speed_bonus;
      if (world->potion_turbo_mode)
      {
        plr_speed += gt->fast_potion_plr_speed_bonus_turbo;
      }
    }
    return plr_speed;
}

void set_enemy_defaults(Enemy *enm)
{
    enm->alive = 1;
    enm->killed = 0;
    enm->move = 0;
    enm->potion = POTION_ID_NONE;
    enm->dx = 1;
    enm->dy = 0;
    enm->reload = 10;
    enm->shots = 1;
    enm->ammo = -1;
    enm->anim = rand() % 15;
    enm->death_animation = 999;
}