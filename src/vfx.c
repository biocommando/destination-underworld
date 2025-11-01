#include "vfx.h"
#include "logging.h"
#include <math.h>

#define calc_sqr_distance(x1, y1, x2, y2) (((x1) - (x2)) * ((x1) - (x2)) + ((y1) - (y2)) * ((y1) - (y2)))

void create_shade_around_hit_point(int x, int y, int roomid, int spread, WorldFx *world_fx)
{
    for (int xx = 0; xx < MAPMAX_X; xx++)
    {
        for (int yy = 0; yy < MAPMAX_Y; yy++)
        {
            int sqr_dist = (int)calc_sqr_distance(x / TILESIZE, y / TILESIZE, xx, yy);
            if (sqr_dist < spread)
            {
                int shade_diff = 4 - sqrt(sqr_dist);
                char *tile_shade = &world_fx->floor_shade_map[roomid - 1][xx][yy];
                *tile_shade += shade_diff > 1 ? shade_diff : 1;
                *tile_shade = *tile_shade > 60 ? 60 : *tile_shade;
            }
        }
    }
}

// Limit the amount of bounce_body_parts calculations within a single frame by
// allowing only one call per map tile
char bounce_body_parts_limit_map[MAPMAX_X][MAPMAX_Y];
static inline int check_bounce_body_parts_limit_map(int x, int y)
{
    WITH_SANITIZED_TILE_COORDINATES(x, y)
    {
        char *limit_map_cell = &bounce_body_parts_limit_map[ok_x][ok_y];
        if (*limit_map_cell)
            return 1;
        *limit_map_cell = 1;
        return 0;
    }
    return 1;
}

static inline void bounce_body_parts(int x, int y, int roomid, WorldFx *world_fx)
{
    if (check_bounce_body_parts_limit_map(x, y))
        return;
    // Make existing bodyparts bounce all over the place
    BodyPartsContainer *bp_container;
    LINKED_LIST_FOR_EACH(&world_fx->bodypart_container, BodyPartsContainer, bp_container, 0)
    {
        if (bp_container->roomid != roomid)
            continue;
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            BodyPart *bp = &bp_container->bodyparts[j];
            if (!bp->exists || bp->velocity >= 20)
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

void create_flame_fx(int x, int y, const World *world, WorldFx *world_fx)
{
    if (get_wall_type_at(world, x, y))
        return;
    if (get_wall_type_at(world, x - 4, y))
        x += 4;
    if (get_wall_type_at(world, x, y - 15))
        y += 15;
    struct flame_fx *f = LINKED_LIST_ADD(&world_fx->flames, struct flame_fx);
    if (world_fx->flames.count > FLAME_FX_COUNT)
    {
        linked_list_remove(&world_fx->flames, world_fx->flames.first);
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

void create_explosion(int x, int y, const World *world, WorldFx *world_fx, double intensity)
{
    const double circle_max_radius = 17;

    Explosion *ex = LINKED_LIST_ADD(&world_fx->explosion, Explosion);
    if (world_fx->explosion.count > 200)
        linked_list_remove(&world_fx->explosion, world_fx->explosion.first);

    ex->x = x - 16 + rand() % 32;
    ex->y = y - 16 + rand() % 32;
    ex->phase = rand() % 5;
    ex->emit_blast_wave = rand() % 5 == 0;

    ex->intensity = intensity;

    ex->circle_count = 5 + rand() % 6;
    double scale = random() * 0.3 + 0.7;
    for (int i = 0; i < ex->circle_count; i++)
    {
        struct explosion_circle *c = &ex->circles[i];
        c->i = random() * 0.25 + 0.75;
        c->i *= intensity;
        c->loc.x = (1 - 2 * random()) * circle_max_radius * scale;
        c->loc.y = (1 - 2 * random()) * circle_max_radius * scale;
        if (rand() % 2)
            c->r = MAX(random() * circle_max_radius * scale, 5);
        else
            c->r = MAX((0.5 + 0.5 * random()) * circle_max_radius * scale, 5);
    }
    // Sort so that most intense are on top (last)
    qsort(ex->circles, ex->circle_count, sizeof(struct explosion_circle), comp_expl_circle);

    bounce_body_parts(x, y, world->current_room, world_fx);
    create_flame_fx(x + rand() % 11 - 5, y + rand() % 11 - 5, world, world_fx);
}

void create_sparkles(int x, int y, int count, int color, int circle_duration, World *world)
{
    for (int i = 0; i < count; i++)
    {
        double angle = 2 * ALLEGRO_PI / count * i;
        double speed = (rand() % 20) / 10.0 + 1;
        struct sparkle_fx *fx = LINKED_LIST_ADD(&world->visual_fx.sparkle_fx, struct sparkle_fx);
        fx->dir.x = sin(angle);
        fx->dir.y = cos(angle);

        fx->loc.x = x + fx->dir.x * 12;
        fx->loc.y = y + fx->dir.y * 12;

        fx->dir.x *= speed;
        fx->dir.y *= speed;

        fx->sprite = rand() % 4;
        fx->duration = 10 + rand() % 10;
        fx->color = color;
    }

    struct sparkle_fx_circle *fxc = LINKED_LIST_ADD(&world->visual_fx.sparkle_fx_circle, struct sparkle_fx_circle);
    fxc->loc.x = x;
    fxc->loc.y = y;
    fxc->duration = circle_duration;
    fxc->time = 0;
    int col = rand() % 3;
    int col_intensity = rand() % 60 + 195;
    fxc->color = al_map_rgb(col == 0 ? col_intensity : 0, col == 1 ? col_intensity : 0, col == 2 ? col_intensity : 0);
}

void spawn_body_parts(const Enemy *enm, WorldFx *world_fx)
{
    BodyPartsContainer *bp_container = LINKED_LIST_ADD(&world_fx->bodypart_container, BodyPartsContainer);
    if (world_fx->bodypart_container.count > BODYPART_CONTAINER_LIMIT)
        linked_list_remove(&world_fx->bodypart_container, world_fx->bodypart_container.first);
    
    bp_container->roomid = enm->roomid;
    for (int j = 0; j < BODYPARTCOUNT; j++)
    {
        BodyPart *bp = &bp_container->bodyparts[j];
        bp->exists = 1;
        bp->type = rand() % 6 + 1;
        bp->x = enm->x;
        bp->y = enm->y;
        bp->anim = rand() % 3;
        double ang = 2 * M_PI * ((double)(rand() % 1000)) / 1000.0;
        bp->dx = sin(ang);
        bp->dy = cos(ang);
        bp->velocity = 20 + rand() % 10;
        bp->dz = j % 2 ? rand() % 15 : rand() % 10;
        if (rand() % 8 == 0)
        {
            bp->dz += 5;
        }
        bp->z = 1;
    }
}

void cleanup_bodyparts(const World *world, WorldFx *world_fx)
{
    memset(bounce_body_parts_limit_map, 0, sizeof(bounce_body_parts_limit_map));

    static int x = 0;
    static int y = 0;

    const Tile *tile = ns_get_tile_at(world, x, y);
    // tile is always non-null
    if (tile->is_floor || tile->is_exit_point || tile->is_exit_level)
    {
        int bp_count = 0;
        BodyPartsContainer *bp_container;
        LINKED_LIST_FOR_EACH(&world_fx->bodypart_container, BodyPartsContainer, bp_container, 0)
        {
            if (bp_container->roomid != world->current_room)
                continue;
            for (int j = 0; j < BODYPARTCOUNT; j++)
            {
                BodyPart *bp = &bp_container->bodyparts[j];
                if (bp->exists &&
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

void stop_bodyparts(WorldFx *world_fx)
{
    BodyPartsContainer *bp_container;
    LINKED_LIST_FOR_EACH(&world_fx->bodypart_container, BodyPartsContainer, bp_container, 0)
    {
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            BodyPart *bp = &bp_container->bodyparts[j];
            bp->dx = 0;
            bp->dy = 0;
            bp->velocity = 0;
            bp->z = 0;
        }
    }
}

void clear_visual_fx(WorldFx *world_fx, int init)
{
    if (init)
    {
        add_managed_list(&world_fx->bodypart_container, world_fx->management_list);
    }
    add_managed_list(&world_fx->explosion, world_fx->management_list);
    add_managed_list(&world_fx->sparkle_fx, world_fx->management_list);
    add_managed_list(&world_fx->sparkle_fx_circle, world_fx->management_list);
    add_managed_list(&world_fx->flames, world_fx->management_list);
    memset(&world_fx->uber_wizard_weapon_fx, 0, sizeof(world_fx->uber_wizard_weapon_fx));

    if (init)
    {
        memset(&world_fx->rune_of_protection_animation, 0, sizeof(world_fx->rune_of_protection_animation));
        memset(&world_fx->hint, 0, sizeof(world_fx->hint));
        memset(&world_fx->floor_shade_map, 0, sizeof(world_fx->floor_shade_map));
        memset(&world_fx->floor_fx, 0, sizeof(world_fx->floor_fx));
    }
}

void create_uber_wizard_weapon_fx(const World *world, WorldFx *world_fx, int x2, int y2, int type)
{
    UberWizardWeaponFx *fx = &world_fx->uber_wizard_weapon_fx;
    fx->start.x = world->plr.x;
    fx->start.y = world->plr.y;
    fx->end.x = x2;
    fx->end.y = y2;
    fx->dim = 20;
    fx->type = type;
}