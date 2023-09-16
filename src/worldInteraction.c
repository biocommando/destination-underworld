#include "logging.h"
#include "worldInteraction.h"
#include <stdio.h>
#include <math.h>
#include "settings.h"
#include "helpers.h"
#include "predictableRandom.h"
#include "iniRead.h"
#include "duscript.h"

extern GameSettings game_settings;

int is_passable(World *world, int x, int y)
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

#define calc_sqr_distance(x1, y1, x2, y2) (((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2)))

void move_enemy(Enemy *enm, World *world)
{
    if (enm->reload > 0)
        enm->reload--;
    enm->wait = enm->reload;

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
    if (enm->dy != 0 && is_passable(world, enm->x, y_check) && is_passable(world, enm->x - THIRDTILESIZE * 1.4, y_check))
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
        world->boss_fight_config->state.positional_trigger_flags |= 1 << t->data;
        LOG_TRACE("pos trigger, new state %x\n", world->boss_fight_config->state.positional_trigger_flags);
    }
}

void create_shade_around_hit_point(int x, int y, int spread, World *world)
{
    for (int xx = 0; xx < MAPMAX_X; xx++)
    {
        for (int yy = 0; yy < MAPMAX_Y; yy++)
        {
            int sqr_dist = (int)calc_sqr_distance(x / TILESIZE, y / TILESIZE, xx, yy);
            if (sqr_dist < spread)
            {
                int shade_diff = 4 - sqrt(sqr_dist);
                char *tile_shade = &world->floor_shade_map[world->current_room - 1][xx][yy];
                *tile_shade += shade_diff > 1 ? shade_diff : 1;
                *tile_shade = *tile_shade > 9 ? 9 : *tile_shade;
            }
        }
    }
}

int tile_check_bullet_hit_wall(World *world, int x, int y)
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
    if (bb->owner_id == NO_OWNER)
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
        bb->owner_id = NO_OWNER;
    }
}
double randomized_bullet_direction(double dir)
{
    return dir + 0.03 - 0.01 * (pr_get_random() % 7);
}

Bullet *get_next_available_bullet(World *world)
{
    for (int i = 0; i < BULLETCOUNT; i++)
    {
        Bullet *bullet = &world->bullets[i];
        if (bullet->owner_id == NO_OWNER)
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

        if (dx != 0 || dy != 0)
        {
            bb->dx = randomized_bullet_direction(dx);
            bb->dy = randomized_bullet_direction(dy);
        }
        else
            return 0;

        bb->x = x;
        bb->y = y;

        bb->owner_id = enm->id;

        bb->hurts_flags = 0;
        if (hurts_monsters)
            bb->hurts_flags |= BULLET_HURTS_MONSTERS;
        if (enm != &world->plr && enm->turret != TURRET_TYPE_PLAYER)
            bb->hurts_flags |= BULLET_HURTS_PLAYER;
        bb->bullet_type = BULLET_TYPE_NORMAL;
    }
    return 1;
}

int shoot_one_shot(Enemy *enm, World *world)
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
    if (bb->owner_id == enm->id)
    {
        return 0;
    }
    if (!(enm->x - HALFTILESIZE < bb->x && enm->x + HALFTILESIZE > bb->x &&
          enm->y - HALFTILESIZE < bb->y && enm->y + HALFTILESIZE > bb->y))
    {
        return 0;
    }

    bb->owner_id = NO_OWNER;

    if (--enm->health <= 0)
    {
        enm->id = NO_OWNER;
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

void bounce_body_parts(int x, int y, World *world)
{
    // Make existing bodyparts bounce all over the place
    for (int i = 0; i < ENEMYCOUNT; i++)
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            BodyPart *bp = &world->enm[i].bodyparts[j];
            if (!bp->exists || world->enm[i].roomid != world->current_room || bp->velocity >= 20)
            {
                continue;
            }
            double distance = calc_sqr_distance(x, y, bp->x, bp->y);
            if (distance < 8100) // 90^2
            {
                distance = sqrt(distance) + 1; // +1 to ensure non-zero divider
                bp->velocity = 250 / (distance + rand() % 50);

                bp->velocity = bp->velocity > 30 ? 30 : bp->velocity;

                bp->dx = (bp->x - x) / distance;
                bp->dy = (bp->y - y) / distance;
                // Prevent body parts from piling up in the same spot
                if (rand() % 10 == 0)
                    bp->dx = -bp->dx;
                if (rand() % 10 == 0)
                    bp->dy = -bp->dy;
            }
        }
}

double random()
{
    return (double)(rand() % 1000) / 1000;
}

int comp_expl_circle(const void *elem1, const void *elem2)
{
    if (((struct explosion_circle *)elem1)->i > ((struct explosion_circle *)elem2)->i)
        return 1;
    return -1;
}

void create_explosion(int x, int y, World *world)
{
    static int explosion_counter = 0;
    const double circle_max_radius = 17;

    Explosion *ex = &world->explosion[explosion_counter];

    ex->x = x - 16 + pr_get_random() % 32;
    ex->y = y - 16 + pr_get_random() % 32;
    ex->phase = pr_get_random() % 5;

    ex->circle_count = 5 + rand() % 6;
    double scale = random() * 0.5 + 0.5;
    for (int i = 0; i < ex->circle_count; i++)
    {
        struct explosion_circle *c = &ex->circles[i];
        c->i = random() * 0.25 + 0.75;
        c->loc.x = 16 + circle_max_radius / 2 + (1 - 2 * random()) * circle_max_radius * scale;
        c->loc.y = 16 + circle_max_radius / 2 + (1 - 2 * random()) * circle_max_radius * scale;
        c->r = MAX(random() * circle_max_radius * scale, 5);
    }
    // Sort so that most intense are on top (last)
    qsort(ex->circles, ex->circle_count, sizeof(struct explosion_circle), comp_expl_circle);

    ex->exists = 1;
    if (++explosion_counter == EXPLOSIONCOUNT)
    {
        explosion_counter = 0;
    }
    bounce_body_parts(x, y, world);
}

void create_sparkles(int x, int y, int count, int color, World *world)
{
    static int sparkle_counter = 0;
    for (int i = 0; i < count; i++)
    {
        double angle = 2 * AL_PI / count * i;
        double speed = (rand() % 20) / 10.0 + 1;
        struct sparkle_fx *fx = &world->sparkle_fx[sparkle_counter];
        fx->dir.x = sin(angle);
        fx->dir.y = cos(angle);

        fx->loc.x = x + fx->dir.x * 12;
        fx->loc.y = y + fx->dir.y * 12;

        fx->dir.x *= speed;
        fx->dir.y *= speed;

        fx->sprite = rand() % 4;
        fx->duration = 10 + rand() % 10;
        fx->color = color;
        if (++sparkle_counter == SPARKLE_FX_COUNT)
        {
            sparkle_counter = 0;
        }
    }
}

Enemy *get_next_available_enemy(World *world, int *index)
{
    int fallback = 1;
    for (int i = 1; i < ENEMYCOUNT; i++)
    {
        if (world->enm[i].former_id == NO_OWNER)
        {
            if (index != NULL)
            {
                *index = i;
            }
            return &world->enm[i];
        }
        if (world->enm[i].id == NO_OWNER)
        {
            fallback = i;
        }
    }
    if (index != NULL)
    {
        *index = fallback;
    }
    return &world->enm[fallback];
}

void clear_map(World *world)
{
    for (int x = 0; x < MAPMAX_X; x++)
    {
        for (int y = 0; y < MAPMAX_Y; y++)
        {
            memset(&world->map[x][y], 0, sizeof(Tile));
        }
    }
}

Enemy *spawn_enemy(int x, int y, int type, int room_id, World *world)
{
    x = x * TILESIZE + HALFTILESIZE;
    y = y * TILESIZE + HALFTILESIZE;
    ns_spawn_enemy(x, y, type, room_id, world);
}

Enemy *ns_spawn_enemy(int x, int y, int type, int room_id, World *world)
{
    int index;
    Enemy *new_enemy = get_next_available_enemy(world, &index);

    new_enemy->id = index + 1;
    new_enemy->former_id = new_enemy->id;
    new_enemy->x = x;
    new_enemy->y = y;
    new_enemy->move = 0;

    new_enemy->sprite = type;

    if (type < 5)
    {
        new_enemy->turret = world->enemy_configs[type].turret ? TURRET_TYPE_ENEMY : TURRET_TYPE_NONE;
        new_enemy->health = world->enemy_configs[type].health;
        new_enemy->rate = world->enemy_configs[type].rate;
        new_enemy->fast = world->enemy_configs[type].fast;
        new_enemy->gold = world->enemy_configs[type].gold;
        new_enemy->hurts_monsters = world->enemy_configs[type].hurts_monsters;
    }
    else if (type == 5)
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

    for (int j = 0; j < BODYPARTCOUNT; j++)
    {
        new_enemy->bodyparts[j].exists = 0;
    }
    new_enemy->roomid = room_id;
    return new_enemy;
}

Potion *get_next_available_potion(World *world)
{
    for (int i = 0; i < POTION_COUNT; i++)
    {
        if (!world->potions[i].exists)
            return &world->potions[i];
    }
    return NULL;
}

Potion *spawn_potion(int x, int y, int type, int room_id, World *world)
{
    Potion *p = get_next_available_potion(world);
    if (!p)
        return NULL;
    p->location.x = x;
    p->location.y = y;
    p->duration_boost = 250;
    if (type == 0)
        p->effects = POTION_EFFECT_SHIELD_OF_FIRE | POTION_EFFECT_ALL_BULLETS_HURT_MONSTERS;
    else if (type == 1)
        p->effects = POTION_EFFECT_STOP_ENEMIES;
    else if (type == 2)
        p->effects = POTION_EFFECT_FAST_PLAYER;
    else if (type == 3)
        p->effects = POTION_EFFECT_BOOSTED_SHOTS;
    else
        p->effects = POTION_EFFECT_HEALING;
    p->sprite = type;
    p->sample = SAMPLE_POTION(type);
    p->room_id = room_id;
    p->exists = 1;
    return p;
}

void combine_tile_properties(Tile *dst, Tile *other)
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

void place_lev_object(World *world, int x, int y, int id, int room_from, int room_to)
{
    Tile tile = create_tile(id);
    if (tile.valid)
    {
        if (tile.is_exit_point && tile.data == room_from)
        {
            world->plr.x = x * TILESIZE + 15;
            world->plr.y = y * TILESIZE + 15;
            if (world->plr.x < 0)
                world->plr.x = 0;
            if (world->plr.x >= 480)
                world->plr.x = 480 - 1;
            if (world->plr.y < 0)
                world->plr.y = 0;
            if (world->plr.y >= 360)
                world->plr.y = 360 - 1;
        }
        if (tile.is_exit_point && tile.data == room_to) // eka huone
        {
            tile = create_tile(TILE_SYM_FLOOR);
        }
        combine_tile_properties(&world->map[x][y], &tile);
        if (tile.data != 0)
        {
            world->map[x][y].data = tile.data;
        }
    }
    else if (id >= 200 && id <= 205)
    {
        if (!world->rooms_visited[room_to - 1])
        {
            spawn_enemy(x, y, id - 200, room_to, world);
        }
    }
    else if (id >= 300 && id <= 305)
    {
        if (!world->rooms_visited[room_to - 1])
        {
            spawn_potion(x * TILESIZE + HALFTILESIZE, y * TILESIZE + HALFTILESIZE, id - 300, room_to, world);
        }
    }
    else
    {
        LOG("ERROR: map object id '%d' not recognized\n", id);
    }
}

void level_read_new_format(World *world, int room_to, FILE *f)
{
    world->boss_fight_config = &world->boss_fight_configs[room_to - 1];
    DuScriptVariable *var;
    int file_start = ftell(f);
    DuScriptState state = du_script_init();
    int boss_exists = world->boss != NULL;

    var = du_script_variable(&state, "level_read");
    strcpy(var->value, world->level_read ? "1" : "0");
    var->read_only = 1;
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
        int id = -1, x = -1, y = -1, room = -1;
        sscanf(buf, "%d %d %d %d", &id, &x, &y, &room);
        if (room_to == room)
        {
            place_lev_object(world, x, y, id, world->current_room, room_to);
        }
    }
    fclose(f);
    if (!world->level_read)
    {
        for (int i = 1; i <= ROOMCOUNT; i++)
        {
            char script_var[16];
            sprintf(script_var, "script%d", i);
            var = du_script_variable(&state, script_var);
            if (*var->value)
            {
                char buf[256];
                sprintf(buf, DATADIR "%s", var->value);
                LOG("Opening bossfight config at %s\n", buf);
                FILE *f2 = fopen(buf, "r");
                if (f2)
                {
                    read_bfconfig(f2, &world->boss_fight_configs[i - 1], world->game_modifiers);
                    fclose(f2);
                    LOG("Bossfight initiated\n");
                    world->boss_fight = 1;
                }
                else
                    LOG("No such file!\n");
            }
        }
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
        char par_var[16];
        sprintf(par_var, "par%d", world->game_modifiers);
        var = du_script_variable(&state, par_var);
        sscanf(var->value, "%lf", &world->par_time);

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
    }

    if (world->boss && !boss_exists)
    {
        world->boss->rate = world->boss_fight_config->fire_rate;
        world->boss->health = world->boss_fight_config->health;
    }

    world->rooms_visited[room_to - 1] = 1;
    world->level_read = 1;
}

int read_level(World *world, int mission, int room_to)
{
    char buf[256];

    int room_from = world->current_room;

    clear_map(world);

    if (!world->level_read)
    {
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
    }

    char mission_name[256];
    sprintf(mission_name, DATADIR "%s\\mission%d", game_settings.mission_pack, mission);

    if (!world->level_read)
    {
        world->boss_fight = 0;
        sprintf(world->mission_display_name, "Mission %d", mission);
        world->story_after_mission_lines = 0;
        world->par_time = 0;
    }
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

void change_room_if_at_exit_point(World *world, int mission)
{
    if (get_tile_at(world, world->plr.x, world->plr.y)->is_exit_point && world->plr.health > 0)
    {
        int to_room = get_tile_at(world, world->plr.x, world->plr.y)->data;
        if (world->plr.roomid != to_room)
        {
            read_level(world, mission, to_room);
            world->current_room = to_room;
            for (int i = 0; i < BULLETCOUNT; i++)
            {
                world->bullets[i].owner_id = NO_OWNER;
            }
            for (int i = 0; i < ENEMYCOUNT; i++)
            {
                if (world->enm[i].roomid == world->current_room &&
                    world->enm[i].id == NO_OWNER && world->enm[i].former_id != NO_OWNER)
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

void read_enemy_configs(World *world)
{
    char fname[256];
    sprintf(fname, DATADIR "%s\\enemy-properties.ini", game_settings.mission_pack);
    FILE *f = fopen(fname, "r");
    for (int i = 0; i < 5; i++)
    {
        char section[10];
        sprintf(section, "type-%d", i);
        world->enemy_configs[i].turret = ini_read_int_value(f, section, "turret");
        world->enemy_configs[i].health = ini_read_int_value(f, section, "health");
        world->enemy_configs[i].rate = ini_read_int_value(f, section, "rate");
        world->enemy_configs[i].gold = ini_read_int_value(f, section, "gold");
        world->enemy_configs[i].fast = ini_read_int_value(f, section, "fast");
        world->enemy_configs[i].hurts_monsters = ini_read_int_value(f, section, "hurts-monsters");
    }
    fclose(f);
}

int parse_highscore_from_world_state(World *world, ArenaHighscore *highscore, int *hs_arena, int *hs_mode)
{
    int arena_idx, mode_idx;
    int mode = world->game_modifiers & (GAMEMODIFIER_DOUBLED_SHOTS | GAMEMODIFIERS_OVER_POWERUP |
                                        GAMEMODIFIER_MULTIPLIED_GOLD | GAMEMODIFIER_BRUTAL);
    for (arena_idx = 0; arena_idx < game_settings.arena_config.number_of_arenas; arena_idx++)
    {
        if (game_settings.arena_config.arenas[arena_idx].level_number == world->mission)
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
