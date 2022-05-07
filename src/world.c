#include "world.h"
#include "predictableRandom.h"
#include "logging.h"
#include <math.h>
#include <stdio.h>

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
    for (int i = 0; i < EXPLOSIONCOUNT; i++)
    {
        world->explosion[i].exists = 0;
    }
    for (int i = 0; i < SPARKLE_FX_COUNT; i++)
    {
        world->sparkle_fx[i].duration = 0;
    }
}

void init_world(World *world)
{
    world->kills = 0;
    for (int i = 0; i < ROOMCOUNT; i++)
    {
        world->rooms_visited[i] = 0;
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 12; y++)
            {
                world->floor_shade_map[i][x][y] = 0;
            }
        }
    }
    clear_visual_fx(world);
    for (int i = 0; i < BULLETCOUNT; i++)
    {
        world->bullets[i].owner_id = NO_OWNER;
    }
    for (int i = 0; i < ENEMYCOUNT; i++)
    {
        Enemy *enm = &world->enm[i];
        enm->id = NO_OWNER;
        enm->former_id = NO_OWNER;
        enm->dx = 1;
        enm->dy = 0;
        enm->reload = 10;
        enm->shots = 1;
        enm->ammo = -1;
        enm->anim = pr_get_random() % 15;
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            enm->bodyparts[j].exists = 0;
        }
    }
    world->boss = NULL;
    world->level_read = 0;
}

Tile create_tile(int symbol)
{
    const int sym_legacy_restriction = 44;
    const int sym_legacy_restriction_clear = 59;
    const int sym_restrcitions_start = 500;
    const int sym_restrcitionClearsStart = 510;
    const int max_restrictions = 10;

    int sym_exit_points_start = TILE_SYM_EXIT_POINT(0);

    Tile t;
    t.flags = TILE_UNRECOGNIZED;
    t.data = 0;
    if (symbol == TILE_SYM_EXIT_LEVEL)
    {
        t.flags |= TILE_IS_EXIT_LEVEL;
        t.flags &= ~TILE_UNRECOGNIZED;
    }
    if ((symbol == TILE_SYM_FLOOR) || (symbol == sym_legacy_restriction) || (symbol == sym_legacy_restriction_clear) ||
        (symbol >= sym_restrcitions_start && symbol < sym_restrcitions_start + 2 * max_restrictions))
    {
        t.flags |= TILE_IS_FLOOR;
        t.flags &= ~TILE_UNRECOGNIZED;
    }
    if (symbol == sym_legacy_restriction_clear) // legacy support
    {
        t.flags |= TILE_IS_CLEAR_RESTRICTION;
    }
    if (symbol >= sym_restrcitionClearsStart && symbol < sym_restrcitionClearsStart + max_restrictions)
    {
        t.flags |= TILE_IS_CLEAR_RESTRICTION;
        t.data = symbol - sym_restrcitionClearsStart;
    }
    if ((symbol == TILE_SYM_WALL1) || (symbol == TILE_SYM_LAVA) || (symbol == TILE_SYM_WALL2) || (symbol == sym_legacy_restriction) ||
        (symbol >= sym_restrcitions_start && symbol < sym_restrcitions_start + max_restrictions))
    {
        t.flags |= TILE_IS_BLOCKER;
        t.flags &= ~TILE_UNRECOGNIZED;
    }
    if (symbol == sym_legacy_restriction) // legacy support
    {
        t.flags |= TILE_IS_RESTRICTED;
    }
    if (symbol >= sym_restrcitions_start && symbol < sym_restrcitions_start + max_restrictions)
    {
        t.flags |= TILE_IS_RESTRICTED;
        t.data = symbol - sym_restrcitions_start;
    }
    if (symbol == TILE_SYM_WALL1)
    {
        t.data = WALL_NORMAL;
        t.flags |= TILE_IS_WALL;
    }
    if (symbol == TILE_SYM_LAVA)
    {
        t.data = WALL_LAVA;
        t.flags |= TILE_IS_WALL;
    }
    if (symbol == TILE_SYM_WALL2)
    {
        t.data = WALL_PENTAGRAM;
        t.flags |= TILE_IS_WALL;
    }
    if (symbol > sym_exit_points_start)
    {
        t.flags |= TILE_IS_EXIT_POINT;
        t.flags &= ~TILE_UNRECOGNIZED;
        t.data = symbol - sym_exit_points_start;
    }
    if (!t.flags)
    {
        t.flags = TILE_IS_FLOOR;
    }
    return t;
}

inline Tile get_tile_at(World *world, int x, int y)
{
    return world->map[x / TILESIZE][y / TILESIZE];
}

inline Tile ns_get_tile_at(World *world, int x, int y)
{
    return world->map[x][y];
}

inline int ns_check_flags_at(World *world, int x, int y, int flags_to_check)
{
    return (world->map[x][y].flags & flags_to_check) != 0;
}

inline int ns_get_wall_type_at(World *world, int x, int y)
{
    Tile *t = &(world->map[x][y]);
    return ((t->flags & TILE_IS_WALL) != 0) ? t->data : 0;
}

inline int check_flags_at(World *world, int x, int y, int flags_to_check)
{
    return ((world->map[x / TILESIZE][y / TILESIZE].flags & flags_to_check) != 0);
}

inline int get_wall_type_at(World *world, int x, int y)
{
    return ns_get_wall_type_at(world, x / TILESIZE, y / TILESIZE);
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

    if (ns_check_flags_at(world, x, y, TILE_IS_FLOOR | TILE_IS_EXIT_POINT | TILE_IS_EXIT_LEVEL))
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

void set_tile_flag(World *world, int x, int y, int flags)
{
    world->map[x / TILESIZE][y / TILESIZE].flags |= flags;
}

void init_player(World *world, Enemy *plrautosave)
{
    Enemy *plr = &world->plr;
    if (plrautosave->id == NO_OWNER)
    {
        // this initializes the player the same way enemy 0 was initialized
        *plr = world->enm[0];
        plr->id = PLAYER_ID;
        plr->former_id = PLAYER_ID;
        plr->sprite = -1;
        plr->shots = 1;
        plr->health = 3;
        plr->completetime = 0;
        plr->rate = 7;
        if ((world->game_modifiers & GAMEMODIFIER_BRUTAL) != 0)
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
