#include "world.h"
#include "predictableRandom.h"
#include <math.h>
#include <stdio.h>

//struct gamedata UniqueGameData;

void stopBodyparts(World *world)
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

void clearExplosions(World *world)
{
    for (int i = 0; i < EXPLOSIONCOUNT; i++)
    {
        world->explosion[i].exists = 0;
    }
}

void initWorld(World *world)
{
    for (int i = 0; i < ROOMCOUNT; i++)
    {
        world->roomsVisited[i] = 0;
        for (int x = 0; x < 16; x++)
        {
            for (int y = 0; y < 12; y++)
            {
                world->floorShadeMap[i][x][y] = 0;
            }
        }
    }
    clearExplosions(world);
    for (int i = 0; i < BULLETCOUNT; i++)
    {
        world->bullets[i].owner_id = NO_OWNER;
    }
    for (int i = 0; i < ENEMYCOUNT; i++)
    {
        Enemy *enm = &world->enm[i];
        enm->id = NO_OWNER;
        enm->formerId = NO_OWNER;
        enm->type = NOT_SET;
        enm->dx = 1;
        enm->dy = 0;
        enm->reload = 10;
        enm->shots = 1;
        enm->ammo = -1;
        enm->anim = prGetRandom() % 15;
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            enm->bodyparts[j].exists = 0;
        }
    }
}

Tile createTile(int symbol)
{
    const int sym_legacyRestriction = 44;
    const int sym_legacyRestrictionClear = 59;
    const int sym_restrcitionsStart = 500;
    const int sym_restrcitionClearsStart = 510;
    const int maxRestrictions = 10;

    int sym_exitPointsStart = TILE_SYM_EXIT_POINT(0);

    Tile t;
    t.flags = TILE_UNRECOGNIZED;
    t.data = 0;
    if (symbol == TILE_SYM_EXIT_LEVEL)
    {
        t.flags |= TILE_IS_EXIT_LEVEL;
        t.flags &= ~TILE_UNRECOGNIZED;
    }
    if ((symbol == TILE_SYM_FLOOR) || (symbol == sym_legacyRestriction) || (symbol == sym_legacyRestrictionClear) ||
        (symbol >= sym_restrcitionsStart && symbol < sym_restrcitionsStart + 2 * maxRestrictions))
    {
        t.flags |= TILE_IS_FLOOR;
        t.flags &= ~TILE_UNRECOGNIZED;
    }
    if (symbol == sym_legacyRestrictionClear) // legacy support
    {
        t.flags |= TILE_IS_CLEAR_RESTRICTION;
    }
    if (symbol >= sym_restrcitionClearsStart && symbol < sym_restrcitionClearsStart + maxRestrictions)
    {
        t.flags |= TILE_IS_CLEAR_RESTRICTION;
        t.data = symbol - sym_restrcitionClearsStart;
    }
    if ((symbol == TILE_SYM_WALL1) || (symbol == TILE_SYM_LAVA) || (symbol == TILE_SYM_WALL2) || (symbol == sym_legacyRestriction) ||
        (symbol >= sym_restrcitionsStart && symbol < sym_restrcitionsStart + maxRestrictions))
    {
        t.flags |= TILE_IS_BLOCKER;
        t.flags &= ~TILE_UNRECOGNIZED;
    }
    if (symbol == sym_legacyRestriction) // legacy support
    {
        t.flags |= TILE_IS_RESTRICTED;
    }
    if (symbol >= sym_restrcitionsStart && symbol < sym_restrcitionsStart + maxRestrictions)
    {
        t.flags |= TILE_IS_RESTRICTED;
        t.data = symbol - sym_restrcitionsStart;
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
    if (symbol > sym_exitPointsStart)
    {
        t.flags |= TILE_IS_EXIT_POINT;
        t.flags &= ~TILE_UNRECOGNIZED;
        t.data = symbol - sym_exitPointsStart;
    }
    if (!t.flags)
    {
        t.flags = TILE_IS_FLOOR;
    }
    return t;
}

Tile getTileAt(World *world, int x, int y)
{
    return world->map[x / TILESIZE][y / TILESIZE];
}

Tile ns_getTileAt(World *world, int x, int y)
{
    return world->map[x][y];
}

int ns_checkFlagsAt(World *world, int x, int y, int flagsToCheck)
{
    return (world->map[x][y].flags & flagsToCheck) != 0;
}

int ns_getWallTypeAt(World *world, int x, int y)
{
    Tile *t = &(world->map[x][y]);
    return ((t->flags & TILE_IS_WALL) != 0) ? t->data : 0;
}

int checkFlagsAt(World *world, int x, int y, int flagsToCheck)
{
    return ((world->map[x / TILESIZE][y / TILESIZE].flags & flagsToCheck) != 0);
}

int getWallTypeAt(World *world, int x, int y)
{
    return ns_getWallTypeAt(world, x / TILESIZE, y / TILESIZE);
}

void spawnBodyParts(Enemy *enm)
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

void setTileFlag(World *world, int x, int y, int flags)
{
     world->map[x / TILESIZE][y / TILESIZE].flags |= flags;
}

void initPlayer(World *world, Enemy *plrautosave)
{
  if (plrautosave->id == NO_OWNER)
  {
    world->plr = world->enm[0];
    world->plr.id = PLAYER_ID;
    world->plr.formerId = PLAYER_ID;
    world->plr.shots = 1;
    world->plr.health = 3;
    world->plr.completetime = 0;
    world->plr.rate = 7;
    if ((world->gameModifiers & GAMEMODIFIER_BRUTAL) != 0)
    {
        world->plr.rate = 12;
    }
    world->plr.ammo = 15;
    world->plr.gold = 0;
    world->plr.type = PLAYER;
  }
  else
  {
    world->plr = *plrautosave;
  }
}
