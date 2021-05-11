#include "worldInteraction.h"
#include <stdio.h>
#include <math.h>
#include "settings.h"
#include "helpers.h"
#include "predictableRandom.h"
#include "iniRead.h"

extern GameSettings game_settings; 

int is_passable(World *world, int x, int y)
{
    return !check_flags_at(world, x, y, TILE_IS_BLOCKER);
}

void clear_restricted_tiles(World *world, int id)
{
    for (int x = 0; x < MAPMAX_X; x++)
    {
        for (int y = 0; y < MAPMAX_Y; y++)
        {
            if (ns_check_flags_at(world, x, y, TILE_IS_RESTRICTED | TILE_IS_CLEAR_RESTRICTION) && ns_get_tile_at(world, x, y).data == id)
            {
                world->map[x][y].flags &= ~(TILE_IS_RESTRICTED | TILE_IS_CLEAR_RESTRICTION | TILE_IS_BLOCKER);
                world->map[x][y].flags |= TILE_IS_FLOOR;
            }
        }
    }
}

double calc_sqr_distance(double x1, double y1, double x2, double y2)
{
    return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
}

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

    if ((enm->x >= world->buf->w || enm->x < 0 || enm->y >= world->buf->h || enm->y < 0) ||
        (enm->id != PLAYER_ID && (check_flags_at(world, enm->x, enm->y, TILE_IS_EXIT_POINT))))
    {
        enm->x = ex;
        enm->y = ey;
    }

    if (enm->id == PLAYER_ID && (check_flags_at(world, enm->x, enm->y, TILE_IS_CLEAR_RESTRICTION)))
    {
        clear_restricted_tiles(world, get_tile_at(world, enm->x, enm->y).data);
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
                world->floor_shade_map[world->current_room - 1][xx][yy] += imax(shade_diff, 1);
                climit(&world->floor_shade_map[world->current_room - 1][xx][yy], 9);
            }
        }
    }
}

void move_bullet(Bullet *bb, World *world)
{
    if (bb->owner_id == NO_OWNER)
        return;
    // Enemy restricts are blockers but not walls, also allows adding e.g. shoot through walls
    if (!check_flags_at(world, (int)bb->x + (int)bb->dx, (int)bb->y, TILE_IS_WALL) &&
        !check_flags_at(world, (int)bb->x, (int)bb->y + (int)bb->dy, TILE_IS_WALL) &&
        !check_flags_at(world, (int)bb->x + (int)bb->dx, (int)bb->y + (int)bb->dy, TILE_IS_WALL) &&
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

int shoot_one_shot_at_xy(double x, double y, double dx, double dy, int enm_id, int hurts_monsters, World *world)
{
    int num_shots = (world->game_modifiers & GAMEMODIFIER_DOUBLED_SHOTS) != 0 ? 2 : 1;
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
    
        bb->owner_id = enm_id;
    
        bb->hurts_monsters = hurts_monsters;
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

    return shoot_one_shot_at_xy(enm->x, enm->y, enm->dx, enm->dy, enm->id, enm->hurts_monsters, world);
}

// Returns 1 if shoot sample should be played
int shoot(Enemy *enm, World *world)
{

    if (enm->reload > 0)
        return 0;

    enm->reload = enm->rate;
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
                bp->velocity = 250 / distance;

                ilimit(&bp->velocity, 30);

                bp->dx = (bp->x - x) / distance;
                bp->dy = (bp->y - y) / distance;
            }
        }
}

void create_explosion(int x, int y, World *world)
{
    static int explosion_counter = 0;

    world->explosion[explosion_counter].x = x - 16 + pr_get_random() % 32;
    world->explosion[explosion_counter].y = y - 16 + pr_get_random() % 32;
    world->explosion[explosion_counter].phase = pr_get_random() % 5;
    world->explosion[explosion_counter].sprite = pr_get_random() % 16;
    world->explosion[explosion_counter].exists = 1;
    if (++explosion_counter == EXPLOSIONCOUNT)
    {
        explosion_counter = 0;
    }
    bounce_body_parts(x, y, world);
}

void create_sparkles(int x, int y, int count, World *world)
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
            world->map[x][y].flags = 0;
            world->map[x][y].data = 0;
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

    new_enemy->id = 1000 * type + index + 1;
    new_enemy->former_id = new_enemy->id;
    new_enemy->x = x;
    new_enemy->y = y;
    new_enemy->move = 0;

    new_enemy->sprite = type;

    if (type < 5)
    {
        new_enemy->turret = world->enemy_configs[type].turret;
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
    
    if (difficulty == DIFFICULTY_BRUTAL) new_enemy->health++; 

    for (int j = 0; j < BODYPARTCOUNT; j++)
    {
        new_enemy->bodyparts[j].exists = 0;
    }
    new_enemy->roomid = room_id;
    return new_enemy;
}

int read_level(World *world, const char *mission_name, int room_to)
{
    char buf[256];

    int room_from = world->current_room;

    clear_map(world);

    world->boss_fight = 0;

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
    fgets(buf, 256, f); // dimensions
    fgets(buf, 256, f);
    int object_count = 0;
    sscanf(buf, "%d", &object_count);

    int boss_exists = world->boss != NULL;

    while (object_count-- > 0)
    {
        fgets(buf, 256, f);
        int id, x, y, room;
        sscanf(buf, "%d %d %d %d", &id, &x, &y, &room);
        if (room == room_to)
        {
            Tile tile = create_tile(id);
            if (!(tile.flags & TILE_UNRECOGNIZED))
            {
                if (tile.flags & TILE_IS_EXIT_POINT && tile.data == room_from)
                {
                    world->plr.x = x * TILESIZE + 15;
                    world->plr.y = y * TILESIZE + 15;
                    if (world->plr.x < 0) world->plr.x = 0;
                    if (world->plr.x >= 480) world->plr.x = 480 - 1;
                    if (world->plr.y < 0) world->plr.y = 0;
                    if (world->plr.y >= 360) world->plr.y = 360 - 1;
                }
                if (tile.flags & TILE_IS_EXIT_POINT && tile.data == room_to) // eka huone
                {
                    tile = create_tile(TILE_SYM_FLOOR);
                }
                world->map[x][y].flags |= tile.flags;
                if (tile.data != 0)
                {
                    world->map[x][y].data = tile.data;
                }
            }
            else if (id >= 200 && id <= 205 && !world->rooms_visited[room_to - 1])
            {
                spawn_enemy(x, y, id - 200, room_to, world);
            }
        }
    }

    int metadata_count = 0;
    fgets(buf, 256, f);
    sscanf(buf, "%d", &metadata_count);
    while (metadata_count-- > 0)
    {
        fgets(buf, 256, f);
        char read_str[64];
        sscanf(buf, "%s", read_str);

        if (!strcmp(read_str, "bossfight")) 
        {
            sscanf(buf, "%*s %s", read_str);
            sprintf(buf, ".\\dataloss\\%s", read_str);
            printf("Opening bossfight config at %s\n", buf);
            FILE *f2 = fopen(buf, "r");
            if (f2)
            {
                read_bfconfig(f2, &world->boss_fight_config);
                fclose(f2);
                printf("Bossfight initiated\n");
                world->boss_fight = 1;
                continue;
            }
            else printf("No such file!\n");
        }
    }

    fclose(f);

    if (world->boss && !boss_exists)
    {
        world->boss->rate = world->boss_fight_config.fire_rate;
        world->boss->health = world->boss_fight_config.health;
    }

    world->rooms_visited[room_to - 1] = 1;
    return 0;
}

void create_cluster_explosion(World *w, double x0, double y0, int num_directions, int intensity, int shoot_id)
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
          shoot_one_shot_at_xy(x, y, dx, dy, shoot_id, shoot_id == PLAYER_ID ? 1 : 0, w);
      }
  }
}

void change_room_if_at_exit_point(World *world, int mission)
{
    if (check_flags_at(world, world->plr.x, world->plr.y, TILE_IS_EXIT_POINT) && world->plr.health > 0)
    {
      int to_room = get_tile_at(world, world->plr.x, world->plr.y).data;
      if (world->plr.roomid != to_room)
      {
        read_level(world, game_settings.missions[mission - 1].filename, to_room);
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
             set_tile_flag(world, world->enm[i].x, world->enm[i].y, TILE_IS_BLOOD_STAINED);
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
    sprintf(fname, ".\\dataloss\\%s\\enemy-properties.ini", game_settings.mission_pack);
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