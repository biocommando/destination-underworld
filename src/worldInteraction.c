#include "logging.h"
#include "worldInteraction.h"
#include <stdio.h>
#include <math.h>
#include "settings.h"
#include "helpers.h"
#include "predictableRandom.h"
#include "vfx.h"
#include "boss_logic.h"
#include "sampleRegister.h"
#include "duColors.h"

static inline int is_passable(const World *world, int x, int y)
{
    return !get_tile_at(world, x, y)->is_blocker;
}

void clear_restricted_tiles(World *world, int id)
{
    for (int x = 0; x < MAPMAX_X; x++)
    {
        for (int y = 0; y < MAPMAX_Y; y++)
        {
            Tile *tile = ns_get_tile_at_mut(world, x, y);
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

inline void enemy_reload(Enemy *enm, int amount)
{
    enm->reload -= amount;
    if (enm->reload < 0)
        enm->reload = 0;
}

void move_enemy(Enemy *enm, World *world)
{
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
            enm->y = TO_PIXEL_COORDINATES((enm->y + TILESIZE / 2) / TILESIZE);
        if (!is_passable(world, x_check, enm->y + THIRDTILESIZE * 1.3))
            enm->y = TO_PIXEL_COORDINATES(enm->y / TILESIZE);
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

    if (enm == &world->plr)
    {
        Tile *t = get_tile_at_mut(world, enm->x, enm->y);
        if (t->is_clear_restriction)
        {
            clear_restricted_tiles(world, t->data);
        }
        if (t->is_positional_trigger)
        {
            pos_trigger_set(world->boss_fight_config->state.positional_trigger_flags, t->data);
            // No point in having the flag set anymore as the condition can trigger only once
            t->is_positional_trigger = 0;
        }
    }
}

static inline int tile_check_bullet_hit_wall(World *world, int x, int y)
{
    Tile *tile = get_tile_at_mut(world, x, y);
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
        create_shade_around_hit_point((int)bb->x, (int)bb->y, world->current_room, 4, &world->visual_fx);
        bb->owner = NULL;
    }
}
static inline double randomized_bullet_direction(double dir)
{
    return dir + 0.03 - 0.01 * (pr_get_random() % 7);
}

Bullet *get_next_available_bullet(World *world)
{
    return LINKED_LIST_ADD(&world->bullets, Bullet);
}

int shoot_one_shot_at_xy(double x, double y, double dx, double dy, Enemy *enm, int hurts_monsters, World *world)
{
    int num_shots = (*world->game_modifiers & GAMEMODIFIER_DOUBLED_SHOTS) != 0 ? 2 : 1;
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
            linked_list_remove_by_value(&world->bullets, bb);
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
    return LINKED_LIST_ADD(&world->enm, Enemy);
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
    x = TO_PIXEL_COORDINATES(x);
    y = TO_PIXEL_COORDINATES(y);
    return ns_spawn_enemy(x, y, type, room_id, world);
}

Enemy *ns_spawn_enemy(int x, int y, int type, int room_id, World *world)
{
    Enemy *new_enemy = get_next_available_enemy(world);

    set_enemy_defaults(new_enemy);
    new_enemy->x = x;
    new_enemy->y = y;

    new_enemy->sprite = type;

    if (type < ENEMY_TYPE_COUNT)
    {
        new_enemy->turret = world->enemy_configs[type].turret ? TURRET_TYPE_ENEMY : TURRET_TYPE_NONE;
        new_enemy->health = world->enemy_configs[type].health;
        new_enemy->rate = world->enemy_configs[type].rate;
        new_enemy->fast = world->enemy_configs[type].fast;
        new_enemy->gold = world->enemy_configs[type].gold;
        new_enemy->hurts_monsters = world->enemy_configs[type].hurts_monsters;
        if (*world->game_modifiers & GAMEMODIFIER_POTION_ON_DEATH)
        {
            new_enemy->potion = world->enemy_configs[type].potion_for_potion_only;
            // Healing potions are "banned" because otherwise the fights will keep on going forever
            if ((*world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT) && new_enemy->potion == POTION_ID_HEAL)
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

    int difficulty = (*world->game_modifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : 0;

    if (difficulty == DIFFICULTY_BRUTAL)
        new_enemy->health++;

    new_enemy->xp = calculate_xp(new_enemy);
    // A little compensation for killing the boss. Doesn't really add much value so late in game.
    if (type == ENEMY_TYPE_COUNT)
        new_enemy->xp += 500;

    new_enemy->roomid = room_id;
    return new_enemy;
}

Potion *spawn_potion(int x, int y, int type, int room_id, World *world, int small)
{
    Potion *p = LINKED_LIST_ADD(&world->potions, Potion);
    if (!p)
        return NULL;
    p->location.x = x;
    p->location.y = y;
    p->duration_boost = POTION_DURATION_BIG_BOOST;
    if (small)
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
            linked_list_clear(&world->bullets);
            for (int x = 0; x < MAPMAX_X; x++)
                for (int y = 0; y < MAPMAX_Y; y++)
                {
                    if (world->visual_fx.floor_fx[world->current_room][x][y] & 1)
                    {
                        ns_get_tile_at_mut(world, x, y)->is_blood_stained = 1;
                    }
                }
            clear_visual_fx(&world->visual_fx, 0);
            stop_bodyparts(&world->visual_fx);
        }
    }
    else
    {
        world->plr.roomid = world->current_room;
    }
}

void set_player_start_state(World *world, GlobalGameState *ggs)
{
    int difficulty = GET_DIFFICULTY(world);
    int excess_gold_limit = difficulty == DIFFICULTY_BRUTAL ? 0 : 5;
    if (ggs->game_modifiers & GAMEMODIFIER_OVERPRICED_POWERUPS)
    {
        excess_gold_limit = difficulty == DIFFICULTY_BRUTAL ? 2 : 7;
    }
    if (ggs->game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD)
    {
        excess_gold_limit = 0;
    }

    world->plr_max_health = (world->plr.perks & PERK_INCREASE_MAX_HEALTH) ? 7 : 6;

    if (world->plr.gold > excess_gold_limit)
    {
        int excess_gold = world->plr.gold - (difficulty == DIFFICULTY_BRUTAL ? 0 : 5);
        if (ggs->game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD)
        {
            excess_gold = world->plr.gold;
        }
        if (ggs->game_modifiers & GAMEMODIFIER_OVERPRICED_POWERUPS)
            excess_gold /= 3;
        world->plr.health += excess_gold * (difficulty == DIFFICULTY_BRUTAL ? 2 : 3);
        world->plr.health = world->plr.health > world->plr_max_health ? world->plr_max_health : world->plr.health;

        if (ggs->game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS)
        {
            world->plr.health *= 3;
        }
    }

    world->plr.gold = world->plr.gold > 5 ? 5 : world->plr.gold;

    if (world->plr.health < 3)
        world->plr.health = 3;
    if (world->plr.ammo < 10)
        world->plr.ammo = 10;

    world->powerups.cluster_strength = 16;

    if ((ggs->game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD) != 0)
    {
        world->powerups.cluster_strength = 5;
        world->plr.gold = 20;
    }
    else if (difficulty == DIFFICULTY_BRUTAL)
        world->plr.gold = 0;
    if (ggs->game_modifiers & GAMEMODIFIER_NO_GOLD)
    {
        world->plr.gold = 0;
        world->plr.ammo = 0;
    }

    if (world->boss_fight && world->boss_fight_config->player_initial_gold >= 0)
    {
        world->plr.gold = world->boss_fight_config->player_initial_gold;
    }

    world->powerups.rune_of_protection_active = 0;

    if (world->plr.perks & PERK_START_WITH_SHIELD_POWERUP)
        world->powerups.rune_of_protection_active = 1;
    if (world->plr.perks & PERK_START_WITH_SPEED_POTION)
    {
        world->potion_effect_flags = POTION_EFFECT_FAST_PLAYER;
        world->potion_duration = POTION_DURATION_BIG_BOOST;
    }
}

inline void apply_timed_potion_effects(World *world)
{

    int potion_effect_divider = world->potion_turbo_mode ? 2 : 1;

    if (world->plr.health > 0 && check_potion_effect(world, POTION_EFFECT_HEALING))
    {
        if (world->potion_healing_counter <= 0)
        {
            if (world->plr.health < world->plr_max_health)
                world->plr.health++;
            world->potion_healing_counter = 25;
        }
        else
        {
            world->potion_healing_counter -= potion_effect_divider;
        }
    }

    if (world->plr.health > 0 && check_potion_effect(world, POTION_EFFECT_SHIELD_OF_FIRE))
    {
        if (world->potion_shield_counter <= 0)
        {
            create_cluster_explosion(world, world->plr.x, world->plr.y, 4, 1, &world->plr);
            world->potion_shield_counter = 15;
        }
        else
        {
            world->potion_shield_counter -= potion_effect_divider;
        }
    }
}

inline Enemy *create_turret(World *world)
{
    Enemy *enm = ns_spawn_enemy(world->plr.x, world->plr.y, 9, world->current_room, world);
    enm->ammo = 128;
    enm->rate = 0;
    enm->shots = 2;
    enm->reload = 10;
    enm->move = 10;
    enm->dx = world->plr.dx;
    enm->dy = world->plr.dy;
    enm->health = 20;
    enm->gold = 0;
    enm->turret = TURRET_TYPE_PLAYER;
    enm->hurts_monsters = 1;
    enm->alive = 1;
    enm->killed = 0;
    return enm;
}

void kill_enemy(Enemy *enm, World *world)
{
    struct killed_enemy_stats *kes = LINKED_LIST_ADD(&world->killed_enemy_stats, struct killed_enemy_stats);
    kes->x = enm->x;
    kes->y = enm->y;
    kes->roomid = enm->roomid;
    kes->ammo = enm->ammo;
    kes->rate = enm->rate;
    kes->shots = enm->shots;
    kes->turret = enm->turret;
    kes->gold = enm->gold;

    enm->health = 0;
    enm->alive = 0;
    enm->killed = 1;
    enm->death_animation = 0;

    create_explosion(enm->x, enm->y, world, &world->visual_fx, 1.8);

    if (check_potion_effect(world, POTION_EFFECT_STOP_ENEMIES))
    {
        world->potion_effect_flags &= ~POTION_EFFECT_STOP_ENEMIES;
        if (!world->potion_effect_flags)
            world->potion_duration = 0;
    }

    world->visual_fx.floor_fx[enm->roomid][enm->x / TILESIZE][enm->y / TILESIZE] |= 1;
    get_tile_at_mut(world, enm->x, enm->y)->is_blood_stained = 1;
    if ((*world->game_modifiers & GAMEMODIFIER_NO_GOLD) == 0)
        world->plr.gold += enm->gold;

    world->kills++;
    world->boss_fight_config->state.player_kills++;
    world->plr.xp += enm->xp;
    if (enm->gold > 0 || (*world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT))
    {
        world->visual_fx.hint.time_shows = 40;
        if (*world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
            sprintf(world->visual_fx.hint.text, "%d", world->kills);
        else if (!(*world->game_modifiers & GAMEMODIFIER_NO_GOLD))
            sprintf(world->visual_fx.hint.text, "+ %d", enm->gold);
        else
            world->visual_fx.hint.time_shows = 0;
        world->visual_fx.hint.loc.x = enm->x - 15;
        world->visual_fx.hint.loc.y = enm->y - 15;
        world->visual_fx.hint.dim = 6;
    }
    if ((*world->game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD) == 0)
    {
        if (world->plr.health < 3 && world->plr.health > 0)
            world->plr.health++;
        world->plr.ammo += 7;
        if (world->plr.ammo > 15)
            world->plr.ammo = 15;
    }
    if (enm->potion >= POTION_ID_SHIELD)
    {
        int potion = enm->potion;
        // if almost out of health, spawn healing potion
        if (world->plr.health == 1)
            potion = POTION_ID_INSTANT_HEAL;
        spawn_potion(enm->x, enm->y, potion, world->current_room, world, 1);
    }

    if (enm == world->boss) // Archmage dies
    {
        LOG_TRACE("boss die logic\n");
        boss_logic(world, 1);
        stop_all_samples();
        trigger_sample_with_params(SAMPLE_BOSSTALK_2, 255, 127 + (enm->x - 240) / 8, 1000);
        for (int xx = 0; xx < 40; xx++)
        {
            int col = xx % 4;
            col = col == 0 ? 255 : (col == 2 ? 64 : 128);
            al_draw_filled_rectangle(0, 0, SCREEN_W + 1, SCREEN_H + 1, GRAY(col));
            al_flip_display();
            wait_delay_ms(25);
        }
        create_cluster_explosion(world, enm->x, enm->y, 48, 1, &world->plr);
    }
    else
    {
        trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (enm->x - 240) / 8, 900 + rand() % 200);
    }
}