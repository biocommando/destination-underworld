#include "worldInteraction.h"
#include <stdio.h>
#include <math.h>
#include "settings.h"
#include "helpers.h"
#include "predictableRandom.h"

//extern struct gamedata UniqueGameData;
extern GameSettings gameSettings; 

int isPassable(World *world, int x, int y)
{
    return !checkFlagsAt(world, x, y, TILE_IS_BLOCKER);
}

void clearRestrictedTiles(World *world, int id)
{
    for (int x = 0; x < MAPMAX_X; x++)
    {
        for (int y = 0; y < MAPMAX_Y; y++)
        {
            if (ns_checkFlagsAt(world, x, y, TILE_IS_RESTRICTED | TILE_IS_CLEAR_RESTRICTION) && ns_getTileAt(world, x, y).data == id)
            {
                world->map[x][y].flags &= ~(TILE_IS_RESTRICTED | TILE_IS_CLEAR_RESTRICTION | TILE_IS_BLOCKER);
                world->map[x][y].flags |= TILE_IS_FLOOR;
            }
        }
    }
}

double calcSqrDistance(double x1, double y1, double x2, double y2)
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
    int xCheck = enm->x + enm->dx * HALFTILESIZE;
    int yCheck = enm->y + enm->dy * HALFTILESIZE;
    if (enm->dx != 0 &&
        isPassable(world, xCheck, enm->y) && isPassable(world, xCheck, enm->y - THIRDTILESIZE) && isPassable(world, xCheck, enm->y + THIRDTILESIZE))
    {
        if (!isPassable(world, xCheck, enm->y - THIRDTILESIZE * 1.3))
            enm->y = (enm->y + TILESIZE / 2) / TILESIZE * TILESIZE + HALFTILESIZE;
        if (!isPassable(world, xCheck, enm->y + THIRDTILESIZE * 1.3))
            enm->y = enm->y / TILESIZE * TILESIZE + HALFTILESIZE;
        enm->x += enm->dx;
        animate = 1;
    }
    if (enm->dy != 0 && isPassable(world, enm->x, yCheck) && isPassable(world, enm->x - THIRDTILESIZE * 1.4, yCheck))
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
        (enm->id != PLAYER_ID && (checkFlagsAt(world, enm->x, enm->y, TILE_IS_EXIT_POINT))))
    {
        enm->x = ex;
        enm->y = ey;
    }

    if (enm->id == PLAYER_ID && (checkFlagsAt(world, enm->x, enm->y, TILE_IS_CLEAR_RESTRICTION)))
    {
        //printf("Clear restr found: %d\n", getTileAt(world, enm->x, enm->y).data);
        clearRestrictedTiles(world, getTileAt(world, enm->x, enm->y).data);
    }
    /*if(enm->id == PLAYER_ID)
    {
        FILE *f = fopen("log.txt", "a");
        fprintf(f,"%d,%d %d\n", enm->x, enm->y, getTileAt(world, enm->x, enm->y).flags);
        fclose(f);
    }*/
}

void createShadeAroundHitPoint(int x, int y, int spread, World *world)
{
    for (int xx = 0; xx < MAPMAX_X; xx++)
    {
        for (int yy = 0; yy < MAPMAX_Y; yy++)
        {
            int sqrDist = (int)calcSqrDistance(x / TILESIZE, y / TILESIZE, xx, yy);
            if (sqrDist < spread)
            {
                int shadeDiff = 4 - sqrt(sqrDist);
                world->floorShadeMap[world->currentRoom - 1][xx][yy] += imax(shadeDiff, 1);
                climit(&world->floorShadeMap[world->currentRoom - 1][xx][yy], 9);
            }
        }
    }
}

void move_bullet(Bullet *bb, World *world)
{
    if (bb->owner_id == NO_OWNER)
        return;
    // Enemy restricts are blockers but not walls, also allows adding e.g. shoot through walls
    if (!checkFlagsAt(world, (int)bb->x + (int)bb->dx, (int)bb->y, TILE_IS_WALL) &&
        !checkFlagsAt(world, (int)bb->x, (int)bb->y + (int)bb->dy, TILE_IS_WALL) &&
        !checkFlagsAt(world, (int)bb->x + (int)bb->dx, (int)bb->y + (int)bb->dy, TILE_IS_WALL) &&
        bb->x < 480 && bb->x > 0 && bb->y < 360 && bb->y > 0)
    {
        bb->x += bb->dx;
        bb->y += bb->dy;
    }
    else
    {
        createShadeAroundHitPoint((int)bb->x, (int)bb->y, 4, world);
        bb->owner_id = NO_OWNER;
    }
}
double randomizedBulletDirection(double dir)
{
    return dir + 0.03 - 0.01 * (prGetRandom() % 7);
}

Bullet *getNextAvailableBullet(World *world)
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

int shootOneShotAtXy(double x, double y, double dx, double dy, int enm_id, int hurtsMonsters, World *world)
{
    int numShots = (world->gameModifiers & GAMEMODIFIER_DOUBLED_SHOTS) != 0 ? 2 : 1;
    for (int i = 0; i < numShots; i++)
    {
        Bullet *bb = getNextAvailableBullet(world);
    
        if (dx != 0 || dy != 0)
        {
            bb->dx = randomizedBulletDirection(dx);
            bb->dy = randomizedBulletDirection(dy);
        }
        else
            return 0;
    
        bb->x = x;
        bb->y = y;
    
        bb->owner_id = enm_id;
/*        if (bb->owner_id == PLAYER_ID)
        {
            UniqueGameData.fireballs++;
        }*/
    
        bb->hurtsMonsters = hurtsMonsters;
        bb->bulletType = BULLET_TYPE_NORMAL;
    }
    return 1;
}

int shootOneShot(Enemy *enm, World *world)
{
    if (enm->ammo == 0)
    {
        return 0;
    }
    else if (enm->ammo > 0)
    {
        enm->ammo--;
    }
    int hurtsMonsters = enm->type == ALIEN_TURRET || enm->type == TURRET || enm->type == PLAYER;

    return shootOneShotAtXy(enm->x, enm->y, enm->dx, enm->dy, enm->id, hurtsMonsters, world);
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
        int ammoLeft = shootOneShot(enm, world);
        if (!ammoLeft)
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
    while (isPassable(world, x, y))
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

void bounceBodyParts(int x, int y, World *world)
{
    // Make existing bodyparts bounce all over the place
    for (int i = 0; i < ENEMYCOUNT; i++)
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            BodyPart *bp = &world->enm[i].bodyparts[j];
            if (!bp->exists || world->enm[i].roomid != world->currentRoom || bp->velocity >= 20)
            {
                continue;
            }
            double distance = calcSqrDistance(x, y, bp->x, bp->y);
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

void createExplosion(int x, int y, World *world)
{
    static int explosionCounter = 0;

    world->explosion[explosionCounter].x = x - 16 + prGetRandom() % 32;
    world->explosion[explosionCounter].y = y - 16 + prGetRandom() % 32;
    world->explosion[explosionCounter].phase = prGetRandom() % 5;
    world->explosion[explosionCounter].sprite = prGetRandom() % 16;
    world->explosion[explosionCounter].exists = 1;
    if (++explosionCounter == EXPLOSIONCOUNT)
    {
        explosionCounter = 0;
    }
    bounceBodyParts(x, y, world);
}

Enemy *getNextAvailableEnemy(World *world, int *index)
{
    int fallback = 1;
    for (int i = 1; i < ENEMYCOUNT; i++)
    {
        if (world->enm[i].formerId == NO_OWNER)
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

void clearMap(World *world)
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

Enemy *spawnEnemy(int x, int y, int type, int roomId, World *world)
{
    int index;
    Enemy *newEnemy = getNextAvailableEnemy(world, &index);
    int enemytype = (type - 200);

    newEnemy->id = 1000 * enemytype + index + 1;
    newEnemy->formerId = newEnemy->id;
    newEnemy->x = x * TILESIZE + HALFTILESIZE;
    newEnemy->y = y * TILESIZE + HALFTILESIZE;
    newEnemy->move = 0;
    newEnemy->rate = 25 - 5 * enemytype;
    newEnemy->health = 2 + enemytype * 3 / 2;
    int difficulty = (world->gameModifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : 0;
    
    if (difficulty == DIFFICULTY_BRUTAL) newEnemy->health++; 

    for (int j = 0; j < BODYPARTCOUNT; j++)
    {
        newEnemy->bodyparts[j].exists = 0;
    }
    if (enemytype == 5)
    {
        newEnemy->rate = world->bossFightConfig.fire_rate;
        /*newEnemy->health = 50;
        if (difficulty == DIFFICULTY_BRUTAL) newEnemy->health = 100;*/
        newEnemy->health = world->bossFightConfig.health;
    }
    newEnemy->roomid = roomId;
    if (enemytype > 1)
        newEnemy->gold = 1;
    else
        newEnemy->gold = 0;
        
    if (newEnemy->id < 1000)
    {
        newEnemy->type = ADEPT;
    }
    else if (newEnemy->id < 2000)
    {
        newEnemy->type = MAGICIAN;
    }
    else if (newEnemy->id < 3000)
    {
        newEnemy->type = IMP;
    }
    else if (newEnemy->id < 4000)
    {
        newEnemy->type = ALIEN;
    }
    else if (newEnemy->id < 5000)
    {
        newEnemy->type = ALIEN_TURRET;
    }
    else if (newEnemy->id < 6000)
    {
        newEnemy->type = ARCH_MAGE;
    }
    else
    {
        newEnemy->type = TURRET;
    }
    return newEnemy;
}

int readLevel(World *world, const char *missionName, int roomTo)
{
    char buf[256];

    int roomFrom = world->currentRoom;

    clearMap(world);

    world->bossFight = 0;
//    world->scripting = 0;

    FILE *f = fopen(missionName, "r");
    if (f == NULL)
    {
        return -1;
    }

    fgets(buf, 256, f); // version
    fgets(buf, 256, f); // dimensions
    fgets(buf, 256, f);
    int objectCount = 0;
    sscanf(buf, "%d", &objectCount);
    while (objectCount-- > 0)
    {
        fgets(buf, 256, f);
        int id, x, y, room;
        sscanf(buf, "%d %d %d %d", &id, &x, &y, &room);
        if (room == roomTo)
        {
            Tile tile = createTile(id);
            if (!(tile.flags & TILE_UNRECOGNIZED))
            {
                if (tile.flags & TILE_IS_EXIT_POINT && tile.data == roomFrom)
                {
                    world->plr.x = x * TILESIZE + 15;
                    world->plr.y = y * TILESIZE + 15;
                    if (world->plr.x < 0) world->plr.x = 0;
                    if (world->plr.x >= 480) world->plr.x = 480 - 1;
                    if (world->plr.y < 0) world->plr.y = 0;
                    if (world->plr.y >= 360) world->plr.y = 360 - 1;
                }
                if (tile.flags & TILE_IS_EXIT_POINT && tile.data == roomTo) // eka huone
                {
                    tile = createTile(TILE_SYM_FLOOR);
                }
                world->map[x][y].flags |= tile.flags;
                if (tile.data != 0)
                {
                    world->map[x][y].data = tile.data;
                }
            }
            else if (id >= 200 && id <= 205 && !world->roomsVisited[roomTo - 1])
            {
                spawnEnemy(x, y, id, roomTo, world);
            }
        }
    }

    int metadataCount = 0;
    fgets(buf, 256, f);
    sscanf(buf, "%d", &metadataCount);
    while (metadataCount-- > 0)
    {
        fgets(buf, 256, f);
        char readStr[64];
        sscanf(buf, "%s", readStr);

/*        if (!strcmp(readStr, "scripting"))
        {
//            world->scripting = 1;
            if (!world->roomsVisited[0])
            {
                sscanf(buf, "%*s %s", readStr);
                sprintf(buf, ".\\dataloss\\%s", readStr);
                if (world->worldScriptInited)
                {
                    freeMemScript(&world->worldScript);
                }
                world->worldScriptInited = 1;
                translateMemScript(buf, &world->worldScript, 32);
            }
            continue;
        }*/
        if (!strcmp(readStr, "bossfight")) 
        {
            sscanf(buf, "%*s %s", readStr);
            sprintf(buf, ".\\dataloss\\%s", readStr);
            printf("Opening bossfight config at %s\n", buf);
            FILE *f2 = fopen(buf, "r");
            if (f2)
            {
                read_bfconfig(f2, &world->bossFightConfig);
                fclose(f2);
                printf("Bossfight initiated\n");
                world->bossFight = 1;
                continue;
            }
            else printf("No such file!\n");
        }
    }

    fclose(f);
    world->roomsVisited[roomTo - 1] = 1;
    return 0;
}

void createClusterExplosion(World *w, double x0, double y0, int num_directions, int intensity, int shoot_id)
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
          shootOneShotAtXy(x, y, dx, dy, shoot_id, shoot_id == PLAYER_ID ? 1 : 0, w);
      }
  }
}

void changeRoomIfAtExitPoint(World *world, int mission)
{
    if (checkFlagsAt(world, world->plr.x, world->plr.y, TILE_IS_EXIT_POINT) && world->plr.health > 0)
    {
      int toRoom = getTileAt(world, world->plr.x, world->plr.y).data;
      if (world->plr.roomid != toRoom)
      {
        readLevel(world, gameSettings.missions[mission - 1].filename, toRoom);
        world->currentRoom = toRoom;
        for (int i = 0; i < BULLETCOUNT; i++)
        {
          world->bullets[i].owner_id = NO_OWNER;
        }
        for (int i = 0; i < ENEMYCOUNT; i++)
        {
          if (world->enm[i].roomid == world->currentRoom && 
              world->enm[i].id == NO_OWNER && world->enm[i].formerId != NO_OWNER)
          {
             setTileFlag(world, world->enm[i].x, world->enm[i].y, TILE_IS_BLOOD_STAINED);
          }
        }
        clearExplosions(world);
        stopBodyparts(world);
      }
    }
    else
    {
      world->plr.roomid = world->currentRoom;
    }
}
