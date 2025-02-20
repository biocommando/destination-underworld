#include "vfx.h"
#include "logging.h"
#include <math.h>

#define calc_sqr_distance(x1, y1, x2, y2) (((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2)))

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

static inline void bounce_body_parts(int x, int y, World *world)
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
            if (distance < 90 * 90)
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

void create_flame_fx_ember(int x, int y, struct flame_ember_fx *f)
{
    f->loc.x = x + rand() % 5 - 2;
    f->loc.y = y;
    int is_red = rand() % 2;
    int col = rand() % 100 + 155;
    f->color = al_map_rgb(col, is_red ? 0 : col, 0);
    f->r = 3 + rand() % 6;
    f->speed = rand() % 2 + 1;
}

void create_flame_fx(int x, int y, World *world)
{
    if (get_tile_at(world, x, y)->is_wall)
        return;
    if (get_tile_at(world, x - 4, y)->is_wall)
        x += 4;
    if (get_tile_at(world, x, y - 15)->is_wall)
        y += 15;
    struct flame_fx *f = world->flames;
    for (int i = 0; i < FLAME_FX_COUNT; i++)
    {
        if (world->flames[i].duration == 0)
        {
            f = &world->flames[i];
            break;
        }
        else if (world->flames[i].duration < f->duration)
        {
            f = &world->flames[i];
        }
    }
    f->loc.x = x;
    f->loc.y = y;
    for (int i = 0; i < EMBERS_PER_FLAME_FX; i++)
    {
        create_flame_fx_ember(x, y, &f->embers[i]);
    }
    f->duration = EMBERS_PER_FLAME_FX * 10 + rand() % 20;
}

static inline int comp_expl_circle(const void *elem1, const void *elem2)
{
    if (((struct explosion_circle *)elem1)->i > ((struct explosion_circle *)elem2)->i)
        return 1;
    return -1;
}

void create_explosion(int x, int y, World *world, double intensity)
{
    static int explosion_counter = 0;
    const double circle_max_radius = 17;

    Explosion *ex = &world->explosion[explosion_counter];

    ex->x = x - 16 + rand() % 32;
    ex->y = y - 16 + rand() % 32;
    ex->phase = rand() % 5;

    ex->intensity = intensity;

    ex->circle_count = 5 + rand() % 6;
    double scale = random() * 0.5 + 0.5;
    for (int i = 0; i < ex->circle_count; i++)
    {
        struct explosion_circle *c = &ex->circles[i];
        c->i = random() * 0.25 + 0.75;
        c->i *= intensity;
        c->loc.x = (1 - 2 * random()) * circle_max_radius * scale;
        c->loc.y = (1 - 2 * random()) * circle_max_radius * scale;
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
    create_flame_fx(x + rand() % 11 - 5, y + rand() % 11 - 5, world);
}

void create_sparkles(int x, int y, int count, int color, int circle_duration, World *world)
{
    static int sparkle_counter = 0;
    static int sparkle_circle_counter = 0;
    for (int i = 0; i < count; i++)
    {
        double angle = 2 * ALLEGRO_PI / count * i;
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

    struct sparkle_fx_circle *fxc = &world->sparkle_fx_circle[sparkle_circle_counter];
    fxc->loc.x = x;
    fxc->loc.y = y;
    fxc->duration = circle_duration;
    fxc->time = 0;
    int col = rand() % 3;
    int col_intensity = rand() % 60 + 195;
    fxc->color = al_map_rgb(col == 0 ? col_intensity : 0, col == 1 ? col_intensity : 0, col == 2 ? col_intensity : 0);

    if (++sparkle_circle_counter == SPARKLE_FX_CIRCLE_COUNT)
    {
        sparkle_circle_counter = 0;
    }
}

void spawn_body_parts(Enemy *enm)
{
    for (int j = 0; j < BODYPARTCOUNT; j++)
    {
        BodyPart *bp = &enm->bodyparts[j];
        bp->exists = 1;
        bp->type = rand() % 6 + 1;
        bp->x = enm->x;
        bp->y = enm->y;
        bp->anim = rand() % 3;
        double ang = 2 * M_PI * ((double)(rand() % 1000)) / 1000.0;
        bp->dx = sin(ang);
        bp->dy = cos(ang);
        bp->velocity = 20 + rand() % 10;
    }
}

void cleanup_bodyparts(World *world)
{
    static int x = 0;
    static int y = 0;

    Tile *tile = ns_get_tile_at(world, x, y);
    if (tile->is_floor || tile->is_exit_point || tile->is_exit_level)
    {
        int bp_count = 0;
        for (int i = 0; i < ENEMYCOUNT; i++)
        {
            for (int j = 0; j < BODYPARTCOUNT; j++)
            {
                BodyPart *bp = &world->enm[i].bodyparts[j];
                if (bp->exists && world->enm[i].roomid == world->current_room &&
                    bp->x > x * TILESIZE && bp->x < (x + 1) * TILESIZE &&
                    bp->y > y * TILESIZE && bp->y < (y + 1) * TILESIZE)
                {
                    if (++bp_count > 40)
                    {
                        bp->exists = 0;
                        LOG_TRACE("Cleanup@%d,%d!\n", x, y);
                    }
                }
            }
        }
    }

    if (++x == MAPMAX_X)
    {
        x = 0;
        if (++y == MAPMAX_Y)
        {
            y = 0;
        }
    }
}

void stop_bodyparts(World *world)
{
    for (int x = 0; x < ENEMYCOUNT; x++)
    {
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            BodyPart *bp = &world->enm[x].bodyparts[j];
            bp->dx = 0;
            bp->dy = 0;
            bp->velocity = 0;
        }
    }
}

void clear_visual_fx(World *world)
{
    memset(world->explosion, 0, sizeof(world->explosion));
    memset(world->sparkle_fx, 0, sizeof(world->sparkle_fx));
    memset(world->sparkle_fx_circle, 0, sizeof(world->sparkle_fx_circle));
    memset(world->flames, 0, sizeof(world->flames));
}