#include <stdio.h>
#include "renderWorld.h"
#include "ducolors.h"
#include "predictableRandom.h"

void draw_enemy(Enemy *enm, World *world)
{

    if (enm->id == PLAYER_ID)
        masked_blit(world->spr, world->buf, 23 * (enm->anim > 20), 0, enm->x - TILESIZE / 2, enm->y - TILESIZE / 2, 23, 29);
    else if (enm->id >= 9000)
        masked_blit(world->spr, world->buf, 47, 117, enm->x - TILESIZE / 2, enm->y - TILESIZE / 2, 24, 28);
    else
        masked_blit(world->spr, world->buf, 23 * (enm->anim > 20), 29 * (enm->id / 1000 + 1), enm->x - TILESIZE / 2, enm->y - TILESIZE / 2, 23, 29);
}

void draw_map(World *world, int col)
{
    const int floorBaseCol = 100; // 66
    const int shadowBaseCol = floorBaseCol - 44;
    
    static int lavaFluctuations = 0;
    if (++lavaFluctuations == 150)
        lavaFluctuations = -149;
    if (col <= 0)
    {
        for (int y = 0; y < 12; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                int fshd = world->floorShadeMap[world->currentRoom - 1][x][y] * 5;
                int shadowcol = shadowBaseCol - 5 * col - fshd;
                shadowcol = shadowcol < 0 ? 0 : shadowcol;
                int floorcol = floorBaseCol - 5 * col - fshd;
                floorcol = floorcol < 0 ? 0 : floorcol;
                int shadowcolm1 = 0;
                if (x > 0)
                {
                    shadowcolm1 = shadowBaseCol - 5 * col - world->floorShadeMap[world->currentRoom - 1][x - 1][y] * 5;
                }
                if (shadowcolm1 < 0)
                {
                    shadowcolm1 = 0;
                }
                int wallType = ns_getWallTypeAt(world, x, y);
                if (wallType == WALL_NORMAL || wallType == WALL_PENTAGRAM)
                {
                    rectfill(world->buf, (x - 1) * TILESIZE, (y)*TILESIZE, (x)*TILESIZE, (y + 1) * TILESIZE - 1, GRAY(shadowcolm1));
                }
                if (ns_checkFlagsAt(world, x, y, TILE_IS_FLOOR | TILE_IS_EXIT_POINT | TILE_IS_EXIT_LEVEL))
                {
                    int drawnColor = GRAY(floorcol);

                    if (ns_checkFlagsAt(world, x, y, TILE_IS_EXIT_POINT))
                    {
                        drawnColor = GRAY(shadowcol);
                    }
                    else if (ns_checkFlagsAt(world, x, y, TILE_IS_EXIT_LEVEL))
                    {
                        drawnColor = makecol(rand() % 32 - 5 * col, rand() % 64 - 5 * col, rand() % 128 - 5 * col);
                    }
                    rectfill(world->buf, (x)*TILESIZE, (y)*TILESIZE, (x + 1) * TILESIZE - 1, (y + 1) * TILESIZE - 1, drawnColor);
                }
                
                if (ns_checkFlagsAt(world, x, y, TILE_IS_BLOOD_STAINED))
                {
                   for(int j = 0; j < 10; j++)
                   {
                       int x_pos = x * TILESIZE + HALFTILESIZE;
                       int y_pos = y * TILESIZE + HALFTILESIZE;
                       int dx = 3 - (x * 13 + y * 7 + j * 3 + world->currentRoom) % 7;
                       int dy = 3 - (j * 13 + x * 7 + y * 3 + world->currentRoom) % 7;
                       for (int i = 0; i < 4; i++)
                       {
                          x_pos += dx;
                          y_pos += dy;
                          rectfill(world->buf, x_pos - 2, y_pos - 2, x_pos + 2, y_pos + 2, makecol(floorcol + 30 - i * 5, 0, 0));
                       }
                   }          
                }
            }
        }
    }
    else
    {

        for (int y = 0; y < 12; y++)
        {
            for (int lev = 15; lev >= 0; lev--)
            {
                int colcalc = lev == 0 ? 165 : (15 - lev) * 10;
                colcalc += y * 10;
                int col_wall = 0;
                if (col == 1)
                {
                    col_wall = makecol((colcalc << 1) / 3, 0, 0);
                }
                else if (col == 2)
                {
                    col_wall = makecol(colcalc >> 3, colcalc >> 1, (colcalc << 2) / 5);
                }
                else if (col == 3)
                {
                    col_wall = makecol((colcalc << 1) / 5, colcalc >> 1, (colcalc << 1) / 5);
                }
                for (int x = 0; x < 16; x++)
                {
                    int wallType = ns_getWallTypeAt(world, x, y);
                    if (wallType)
                    {
                        if (wallType == WALL_NORMAL || wallType == WALL_PENTAGRAM)
                        {
                            rectfill(world->buf, x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 14, y * TILESIZE + lev + 14, col_wall);
                        }
                        if (wallType == WALL_LAVA && (((int)abs(lavaFluctuations) / 5 + (y & x) * (y | x)) % 15 == lev || lev == 15))
                        {
                            rectfill(world->buf, x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 14, y * TILESIZE + lev + 14, makecol(colcalc, colcalc >> 1, colcalc >> 2));
                        }
                    }
                    if (lev == 0)
                    {
                        if (wallType == WALL_PENTAGRAM)
                        {
                            masked_blit(world->spr, world->buf, 47, 145, x * TILESIZE - 15, y * TILESIZE - 15, 30, 30);
                        }
                        else if (ns_checkFlagsAt(world, x, y, TILE_IS_EXIT_POINT))
                        {
                            rectfill(world->buf, x * TILESIZE - 15, y * TILESIZE - 15, x * TILESIZE + 14, y * TILESIZE + 14, GRAY(100));
                            rectfill(world->buf, x * TILESIZE - 10, y * TILESIZE - 10, x * TILESIZE + 9, y * TILESIZE + 9, GRAY(90));
                            rectfill(world->buf, x * TILESIZE - 5, y * TILESIZE - 5, x * TILESIZE + 4, y * TILESIZE + 4, GRAY(80));
                        }
                    }
                }
            }
        }
    }
}

void drawPlayerLegend(World *world)
{
    if (world->plr.health > 0)
    {
        for (int x = 0; x < world->plr.health; x++)
            masked_blit(world->spr, world->buf, 60, 0, world->plr.x - 23, world->plr.y - 18 + 4 * x, 7, 6);
        for (int x = 0; x < world->plr.ammo; x++)
        {
            if (world->plr.reload == 0)
            {
             masked_blit(world->spr, world->buf, 67, 0, world->plr.x + 10, world->plr.y - 18 + 2 * x, 6, 3);
            }
            else
            {
             masked_blit(world->spr, world->buf, 73, 0, world->plr.x + 10, world->plr.y - 18 + 2 * x, 6, 3);
            }
        }

        masked_blit(world->spr, world->buf, 89 + (world->plr.shots > 1) * 4, 196, world->plr.x - 21, world->plr.y - 14 + 4 * world->plr.health, 3, 6);
        if (world->plr.gold > 9)
        {
            masked_blit(world->spr, world->buf, 49 + (world->plr.gold / 10) * 4, 196, world->plr.x - 25, world->plr.y - 5 + 4 * world->plr.health, 3, 6);
        }
        masked_blit(world->spr, world->buf, 49 + (world->plr.gold % 10) * 4, 196, world->plr.x - 21, world->plr.y - 5 + 4 * world->plr.health, 3, 6);
    }
}

void moveAndDrawBodyParts(World *world)
{
    for (int x = 0; x < ENEMYCOUNT; x++)
    {
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            int bonesturn = 0;
            BodyPart *bodypart = &world->enm[x].bodyparts[j];
            if (bodypart->exists && world->enm[x].roomid == world->currentRoom)
            {
                if (bodypart->velocity > 0)
                {
                    for (int travel_amt = bodypart->velocity; travel_amt > 0; travel_amt--)
                    {
                        for (int blood_trail_idx = 1; blood_trail_idx <= 3; blood_trail_idx++)
                        {
                            masked_blit(world->spr, world->buf, 334 + rand() % 18, 129 + rand() % 18,
                                        (int)bodypart->x - blood_trail_idx * bodypart->dx - 8 + rand() % 4, (int)bodypart->y - blood_trail_idx * bodypart->dx - 8 + rand() % 4, 2, 2);
                        }
                        
                        bodypart->x += bodypart->dx;
                        if (getWallTypeAt(world, bodypart->x, bodypart->y))
                        {
                            bodypart->dx = -bodypart->dx;
                        }
                        bodypart->y += bodypart->dy;
                        if (getWallTypeAt(world, bodypart->x, bodypart->y))
                        {
                            bodypart->dy = -bodypart->dy;
                        }
                    }

                    bodypart->velocity--;
                    bonesturn = 1;
                }

                if (bodypart->anim == 4)
                {
                    bodypart->anim = 0;
                }
                else if (bonesturn)
                {
                    bodypart->anim++;
                }

                masked_blit(world->spr, world->buf, 311 + 11 * (bodypart->anim > 1),
                            129 + 11 * (bodypart->type - 1), (int)bodypart->x - 5, (int)bodypart->y - 5, 11, 11);
            }
        }
    }
}

int progressAndDrawExplosions(World *world)
{
    int vibrations = 0;
    for (int x = 0; x < EXPLOSIONCOUNT; x++)
    {
        Explosion *ex = &world->explosion[x];
        if (!ex->exists)
            continue;
        vibrations++;
        //masked_blit(world->spr, world->buf, 100 + (ex->sprite) * 32, (ex->phase / 10) * 32, ex->x - 16, ex->y - 16, 32, 32);
        masked_blit(world->explosSpr, world->buf, (ex->sprite) * 64, (ex->phase / 10) * 64, ex->x - 32, ex->y - 32, 64, 64);
        ex->phase += 8;
        if (ex->phase >= 240)
        {
            ex->exists = 0;
        }
    }
    return vibrations;
}

void displayLevelInfo(World *world, int mission, int missionCount, BITMAP *bmp_levclear, FONT *font)
{
    clear_to_color(world->buf, 0);
    blit(bmp_levclear, world->buf, 0, 0, 0, world->buf->h - bmp_levclear->h, bmp_levclear->w, bmp_levclear->h);
    textprintf_ex(world->buf, font, 5, 5, WHITE, -1, "Level cleared!");
    textprintf_ex(world->buf, font, 5, 30, WHITE, -1, "Now entering level %d / %d.", mission + 1, missionCount);
/*    textprintf_ex(world->buf, font, 5, 45, WHITE, -1, "Current game total stats:");
    textprintf_ex(world->buf, font, 5, 60, WHITE, -1, "Deaths: %d", uniqueData->deaths);
    textprintf_ex(world->buf, font, 5, 75, WHITE, -1, "Kills: %d", uniqueData->kills);
    textprintf_ex(world->buf, font, 5, 90, WHITE, -1, "Fireballs fired: %d", uniqueData->fireballs);
    if (uniqueData->deaths > 0)
    {
        textprintf_ex(world->buf, font, 200, 75, WHITE, -1, "Kill/death ratio: %.2f", (float)uniqueData->kills / uniqueData->deaths);
    }
    textprintf_ex(world->buf, font, 200, 90, WHITE, -1, "Fireball/kill ratio: %.2f", (float)uniqueData->fireballs / uniqueData->kills);
    textprintf_ex(world->buf, font, 5, 105, WHITE, -1, "Power ups used: %d", uniqueData->powerups);*/
    textprintf_ex(world->buf, font, 5, 120, WHITE, -1, "Press enter to continue!");
    stretch_blit(world->buf, screen, 0, 0, 480, 360, 0, 0, screen->w, screen->h);
}

void show_gold_hint(World *world, char *hintText, int *hintX, int *hintY, int *hintDim, int *hint_timeShows, int number)
{
  sprintf(hintText, "- %d", number);
  *hintX = world->plr.x - 15;
  *hintY = world->plr.y - 30;
  *hintDim = 4;
  *hint_timeShows = 60;
}
