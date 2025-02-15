#include "logging.h"
#include "worldInteraction.h"
#include <stdio.h>
#include <math.h>
#include "settings.h"
#include "helpers.h"
#include "predictableRandom.h"
#include "vfx.h"

static inline int is_passable(World *world, int x, int y)
{
    return !get_tile_at(world, x, y)->is_blocker;
}

void clear_restricted_tiles(World *world, int id)
{
    for (int x = 0; x < MAPMAX_X; x++)
    {
        for (int y = 0; y < MAPMAX_Y; y++)
        {
            Tile *tile = ns_get_tile_at(world, x, y);
            if ((tile->is_restricted || tile->is_clear_restriction) && tile->data == id)
            {
                tile->is_restricted = 0;
                tile->is_clear_restriction = 0;
                tile->is_blocker = 0;
                tile->is_floor = 1;
            }
        }
    }
}

void move_enemy(Enemy *enm, World *world)
{
    if (enm->reload > 0)
        enm->reload--;

    if (!enm->move)
        return;
    int animate = 0;
    int ex = enm->x, ey = enm->y;
    int x_check = enm->x + enm->dx * HALFTILESIZE;
    int y_check = enm->y + enm->dy * HALFTILESIZE;
    if (enm->dx != 0 &&
        is_passable(world, x_check, enm->y) && is_passable(world, x_check, enm->y - THIRDTILESIZE) && is_passable(world, x_check, enm->y + THIRDTILESIZE))
    {
        if (!is_passable(world, x_check, enm->y - THIRDTILESIZE * 1.3))
            enm->y = (enm->y + TILESIZE / 2) / TILESIZE * TILESIZE + HALFTILESIZE;
        if (!is_passable(world, x_check, enm->y + THIRDTILESIZE * 1.3))
            enm->y = enm->y / TILESIZE * TILESIZE + HALFTILESIZE;
        enm->x += enm->dx;
        animate = 1;
    }
    if (enm->dy != 0 && is_passable(world, enm->x, y_check) && is_passable(world, enm->x - THIRDTILESIZE, y_check) && is_passable(world, enm->x + THIRDTILESIZE, y_check))
    {
        enm->y += enm->dy;
        animate = 1;
    }

    enm->anim += animate;
    if (enm->anim > ANIM_FRAME_COUNT)
    {
        enm->anim = 0;
    }

    if ((enm->x >= SCREEN_W || enm->x < 0 || enm->y >= SCREEN_H || enm->y < 0) ||
        (enm != &world->plr && (get_tile_at(world, enm->x, enm->y)->is_exit_point)))
    {
        enm->x = ex;
        enm->y = ey;
    }

    Tile *t = get_tile_at(world, enm->x, enm->y);
    if (enm == &world->plr && t->is_clear_restriction)
    {
        clear_restricted_tiles(world, t->data);
    }
    if (enm == &world->plr && t->is_positional_trigger)
    {
        LOG_TRACE("pos trigger current state %x\n", world->boss_fight_config->state.positional_trigger_flags);
        world->boss_fight_config->state.positional_trigger_flags |= 1 << t->data;
        LOG_TRACE("pos trigger, new state %x\n", world->boss_fight_config->state.positional_trigger_flags);
    }
}

static inline int tile_check_bullet_hit_wall(World *world, int x, int y)
{
    Tile *tile = get_tile_at(world, x, y);
    if (tile->is_wall)
    {
        if (tile->durability > 0)
        {
            tile->durability--;
            if (tile->durability == 0)
            {
                *tile = create_tile(TILE_SYM_FLOOR);
            }
        }
        return 1;
    }
    return 0;
}

void move_bullet(Bullet *bb, World *world)
{
    if (bb->owner == NULL)
        return;
    // Enemy restricts are blockers but not walls, also allows adding e.g. shoot through walls
    if (!tile_check_bullet_hit_wall(world, bb->x + (int)bb->dx, (int)bb->y) &&
        !tile_check_bullet_hit_wall(world, bb->x, bb->y + bb->dy) &&
        !tile_check_bullet_hit_wall(world, bb->x + (int)bb->dx, (int)bb->y + (int)bb->dy) &&
        bb->x < 480 && bb->x > 0 && bb->y < 360 && bb->y > 0)
    {
        bb->x += bb->dx;
        bb->y += bb->dy;
    }
    else
    {
        create_shade_around_hit_point((int)bb->x, (int)bb->y, 4, world);
        bb->owner = NULL;
    }
}
static inline double randomized_bullet_direction(double dir)
{
    return dir + 0.03 - 0.01 * (pr_get_random() % 7);
}

Bullet *get_next_available_bullet(World *world)
{
    for (int i = 0; i < BULLETCOUNT; i++)
    {
        Bullet *bullet = &world->bullets[i];
        if (bullet->owner == NULL)
        {
            return bullet;
        }
    }
    return world->bullets;
}

int shoot_one_shot_at_xy(double x, double y, double dx, double dy, Enemy *enm, int hurts_monsters, World *world)
{
    int num_shots = (world->game_modifiers & GAMEMODIFIER_DOUBLED_SHOTS) != 0 ? 2 : 1;
    if (enm == &world->plr && check_potion_effect(world, POTION_EFFECT_BOOSTED_SHOTS))
    {
        num_shots *= 2;
    }
    if (check_potion_effect(world, POTION_EFFECT_ALL_BULLETS_HURT_MONSTERS))
    {
        hurts_monsters = 1;
    }
    for (int i = 0; i < num_shots; i++)
    {
        Bullet *bb = get_next_available_bullet(world);

        bb->duration = 0;

        if (dx != 0 || dy != 0)
        {
            bb->dx = randomized_bullet_direction(dx);
            bb->dy = randomized_bullet_direction(dy);
        }
        else
        {
            return 0;
        }

        bb->x = x;
        bb->y = y;

        bb->owner = enm;

        bb->hurts_flags = 0;
        if (hurts_monsters)
            bb->hurts_flags |= BULLET_HURTS_MONSTERS;
        if (enm != &world->plr && enm->turret != TURRET_TYPE_PLAYER)
            bb->hurts_flags |= BULLET_HURTS_PLAYER;
        bb->bullet_type = BULLET_TYPE_NORMAL;
    }
    return 1;
}

static inline int shoot_one_shot(Enemy *enm, World *world)
{
    if (enm->ammo == 0)
    {
        return 0;
    }
    else if (enm->ammo > 0)
    {
        enm->ammo--;
    }

    return shoot_one_shot_at_xy(enm->x, enm->y, enm->dx, enm->dy, enm, enm->hurts_monsters, world);
}

// Returns 1 if shoot sample should be played
int shoot(Enemy *enm, World *world)
{

    if (enm->reload > 0)
        return 0;

    enm->reload = enm->rate;
    if (enm == &world->plr && check_potion_effect(world, POTION_EFFECT_FAST_PLAYER))
    {
        enm->reload /= 2;
    }
    int sample_plays = 0;
    for (int i = 0; i < enm->shots; i++)
    {
        int ammo_left = shoot_one_shot(enm, world);
        if (!ammo_left)
        {
            break;
        }
        sample_plays = 1;
    }
    return sample_plays;
}

int bullet_hit(Enemy *enm, Bullet *bb)
{
    if (bb->owner == enm)
    {
        return 0;
    }
    if (!(enm->x - HALFTILESIZE < bb->x && enm->x + HALFTILESIZE > bb->x &&
          enm->y - HALFTILESIZE < bb->y && enm->y + HALFTILESIZE > bb->y))
    {
        return 0;
    }

    bb->owner = NULL;

    if (--enm->health <= 0)
    {
        enm->alive = 0;
        enm->killed = 1;
        enm->death_animation = 0;
    }

    return 1;
}

int sees_each_other(Enemy *e1, Enemy *e2, World *world)
{
    double x = e1->x;
    double y = e1->y;
    // 32 is just a random number we use for scaling the vector
    // from one Enemy to another (for some reason not calculating
    // a proper unit vector, maybe some stupid optimization?)
    double dx = (double)(e2->x - e1->x) / 32;
    double dy = (double)(e2->y - e1->y) / 32;
    while (is_passable(world, x, y))
    {
        x += dx;
        y += dy;
        if (x > e2->x - 1 && x < e2->x + 1 && y > e2->y - 1 && y < e2->y + 1)
        {
            return 1;
        }
    }
    return 0;
}

Enemy *get_next_available_enemy(World *world)
{
    int fallback = 0;
    for (int i = 0; i < ENEMYCOUNT; i++)
    {
        if (!world->enm[i].alive)
        {
            if (!world->enm[i].killed)
            {
                return &world->enm[i];
            }
            fallback = i;
        }
    }
    return &world->enm[fallback];
}

static inline int calculate_xp(Enemy *enm)
{
    int rate = 30 - enm->rate;
    if (rate < 0)
        rate = 0;
    double power_score = rate / 5 + enm->health / 2 + enm->fast * 2 - enm->turret - enm->hurts_monsters;
    return pow(power_score, 1.5);
}

Enemy *spawn_enemy(int x, int y, int type, int room_id, World *world)
{
    x = x * TILESIZE + HALFTILESIZE;
    y = y * TILESIZE + HALFTILESIZE;
    ns_spawn_enemy(x, y, type, room_id, world);
}

Enemy *ns_spawn_enemy(int x, int y, int type, int room_id, World *world)
{
    Enemy *new_enemy = get_next_available_enemy(world);

    new_enemy->alive = 1;
    new_enemy->killed = 0;
    new_enemy->x = x;
    new_enemy->y = y;
    new_enemy->move = 0;
    new_enemy->potion = POTION_ID_NONE;

    new_enemy->sprite = type;

    if (type < ENEMY_TYPE_COUNT)
    {
        new_enemy->turret = world->enemy_configs[type].turret ? TURRET_TYPE_ENEMY : TURRET_TYPE_NONE;
        new_enemy->health = world->enemy_configs[type].health;
        new_enemy->rate = world->enemy_configs[type].rate;
        new_enemy->fast = world->enemy_configs[type].fast;
        new_enemy->gold = world->enemy_configs[type].gold;
        new_enemy->hurts_monsters = world->enemy_configs[type].hurts_monsters;
        if (world->game_modifiers & GAMEMODIFIER_POTION_ON_DEATH)
        {
            new_enemy->potion = world->enemy_configs[type].potion_for_potion_only;
            // Healing potions are "banned" because otherwise the fights will keep on going forever
            if ((world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT) && new_enemy->potion == POTION_ID_HEAL)
            {
                new_enemy->potion = POTION_ID_INSTANT_HEAL;
            }
        }
    }
    else if (type == ENEMY_TYPE_COUNT)
    {
        world->boss = new_enemy;
        new_enemy->hurts_monsters = 0;
        new_enemy->turret = 0;
        new_enemy->gold = 0;
        // fast is overridden by boss speed
        // rate and health are set in boss config
    }

    int difficulty = (world->game_modifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : 0;

    if (difficulty == DIFFICULTY_BRUTAL)
        new_enemy->health++;

    new_enemy->xp = calculate_xp(new_enemy);

    for (int j = 0; j < BODYPARTCOUNT; j++)
    {
        new_enemy->bodyparts[j].exists = 0;
    }
    new_enemy->roomid = room_id;
    return new_enemy;
}

Potion *get_next_available_potion(World *world, int range_start, int range_end)
{
    for (int i = range_start; i < range_end; i++)
    {
        if (!world->potions[i].exists)
            return &world->potions[i];
    }
    // If dropped potions run out, start reusing from oldest
    if (range_start != POTION_PRESET_RANGE_START)
    {
        int min_exists = range_start;
        for (int i = range_start + 1; i < range_end; i++)
        {
            if (world->potions[i].exists < world->potions[min_exists].exists)
                min_exists = i;
        }
        return &world->potions[min_exists];
    }
    return NULL;
}

Potion *spawn_potion(int x, int y, int type, int room_id, World *world, int range_start, int range_end)
{
    static int spawned_count = 0;
    Potion *p = get_next_available_potion(world, range_start, range_end);
    if (!p)
        return NULL;
    p->location.x = x;
    p->location.y = y;
    p->duration_boost = POTION_DURATION_BIG_BOOST;
    int is_drop = range_start != POTION_PRESET_RANGE_START;
    if (is_drop)
        p->duration_boost = POTION_DURATION_MINI_BOOST;
    p->effects = 0;
    if (type == POTION_ID_SHIELD)
        p->effects = POTION_EFFECT_SHIELD_OF_FIRE | POTION_EFFECT_ALL_BULLETS_HURT_MONSTERS;
    else if (type == POTION_ID_STOP)
        p->effects = POTION_EFFECT_STOP_ENEMIES;
    else if (type == POTION_ID_FAST)
        p->effects = POTION_EFFECT_FAST_PLAYER;
    else if (type == POTION_ID_BOOST)
        p->effects = POTION_EFFECT_BOOSTED_SHOTS;
    else if (type == POTION_ID_HEAL)
        p->effects = POTION_EFFECT_HEALING;
    else if (type == POTION_ID_MINOR_SHIELD)
        p->effects = POTION_EFFECT_SHIELD_OF_FIRE;
    else
        p->effects = POTION_EFFECT_HEAL_ONCE;
    p->sprite = type;
    p->sample = SAMPLE_POTION(type);
    p->room_id = room_id;
    p->exists = ++spawned_count;
    return p;
}

void create_cluster_explosion(World *w, double x0, double y0, int num_directions, int intensity, Enemy *enm)
{
    double half_dirs = num_directions / 2;
    for (int i = 0; i < num_directions; i++)
    {
        double x = x0;
        double y = y0;
        double dx = sin(M_PI * i / half_dirs) * 0.5;
        double dy = cos(M_PI * i / half_dirs) * 0.5;
        for (int bidx = 0; bidx < intensity; bidx++)
        {
            x += dx * 0.66;
            y += dy * 0.66;
            shoot_one_shot_at_xy(x, y, dx, dy, enm, enm == &w->plr ? 1 : 0, w);
        }
    }
}

static void place_player_at_entrance(World *world, int to_room)
{
    for (int x = 0; x < MAPMAX_X; x++)
    {
        for (int y = 0; y < MAPMAX_Y; y++)
        {
            Tile *t = &world->map[to_room - 1][x][y];
            if (t->is_exit_point && t->data == world->current_room)
            {
                world->plr.x = x * TILESIZE + 15;
                world->plr.y = y * TILESIZE + 15;
                return;
            }
        }
    }
    LOG("ERROR: exit point not found!\n");
}

void change_room_if_at_exit_point(World *world)
{
    if (get_tile_at(world, world->plr.x, world->plr.y)->is_exit_point && world->plr.health > 0)
    {
        int to_room = get_tile_at(world, world->plr.x, world->plr.y)->data;
        if (world->plr.roomid != to_room)
        {
            world->boss_fight_config = &world->boss_fight_configs[to_room - 1];
            LOG_TRACE("Bossfight events: %d\n", world->boss_fight_config->num_events);
            place_player_at_entrance(world, to_room);
            world->current_room = to_room;
            for (int i = 0; i < BULLETCOUNT; i++)
            {
                world->bullets[i].owner = NULL;
            }
            for (int i = 0; i < ENEMYCOUNT; i++)
            {
                if (world->enm[i].roomid == world->current_room &&
                    !world->enm[i].alive && world->enm[i].killed)
                {
                    get_tile_at(world, world->enm[i].x, world->enm[i].y)->is_blood_stained = 1;
                }
            }
            clear_visual_fx(world);
            stop_bodyparts(world);
        }
    }
    else
    {
        world->plr.roomid = world->current_room;
    }
}

int parse_highscore_from_world_state(const World *world, ArenaHighscore *highscore, int *hs_arena, int *hs_mode)
{
    int arena_idx, mode_idx;
    int mode = world->game_modifiers & (~GAMEMODIFIER_ARENA_FIGHT);
    for (arena_idx = 0; arena_idx < get_game_settings()->arena_config.number_of_arenas; arena_idx++)
    {
        if (get_game_settings()->arena_config.arenas[arena_idx].level_number == world->mission)
            break;
    }
    int highscore_kills = 0;
    for (mode_idx = 0; mode_idx < ARENACONF_HIGHSCORE_MAP_SIZE; mode_idx++)
    {
        if (highscore->mode[arena_idx][mode_idx] == mode)
        {
            highscore_kills = highscore->kills[arena_idx][mode_idx];
            break;
        }
    }
    if (mode_idx == ARENACONF_HIGHSCORE_MAP_SIZE)
    {
        for (mode_idx = 0; mode_idx < ARENACONF_HIGHSCORE_MAP_SIZE; mode_idx++)
        {
            if (highscore->mode[arena_idx][mode_idx] == -1)
            {
                // Reserve this index in the map for the new entry
                highscore->mode[arena_idx][mode_idx] = mode;
                break;
            }
        }
    }
    *hs_arena = arena_idx;
    *hs_mode = mode_idx;
    return highscore_kills;
}
