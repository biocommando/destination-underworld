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
    const int floor_base_col = 100; // 66
    const int shadow_base_col = floor_base_col - 44;
    
    static int lava_fluctuations = 0;
    if (++lava_fluctuations == 150)
        lava_fluctuations = -149;
    if (col <= 0)
    {
        for (int y = 0; y < 12; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                int fshd = world->floor_shade_map[world->current_room - 1][x][y] * 5;
                int shadowcol = shadow_base_col - 5 * col - fshd;
                shadowcol = shadowcol < 0 ? 0 : shadowcol;
                int floorcol = floor_base_col - 5 * col - fshd;
                floorcol = floorcol < 0 ? 0 : floorcol;
                int shadowcolm1 = 0;
                if (x > 0)
                {
                    shadowcolm1 = shadow_base_col - 5 * col - world->floor_shade_map[world->current_room - 1][x - 1][y] * 5;
                }
                if (shadowcolm1 < 0)
                {
                    shadowcolm1 = 0;
                }
                int wall_type = ns_getWallTypeAt(world, x, y);
                if (wall_type == WALL_NORMAL || wall_type == WALL_PENTAGRAM)
                {
                    rectfill(world->buf, (x - 1) * TILESIZE, (y)*TILESIZE, (x)*TILESIZE, (y + 1) * TILESIZE - 1, GRAY(shadowcolm1));
                }
                if (ns_check_flags_at(world, x, y, TILE_IS_FLOOR | TILE_IS_EXIT_POINT | TILE_IS_EXIT_LEVEL))
                {
                    int drawn_color = GRAY(floorcol);

                    if (ns_check_flags_at(world, x, y, TILE_IS_EXIT_POINT))
                    {
                        drawn_color = GRAY(shadowcol);
                    }
                    else if (ns_check_flags_at(world, x, y, TILE_IS_EXIT_LEVEL))
                    {
                        drawn_color = makecol(rand() % 32 - 5 * col, rand() % 64 - 5 * col, rand() % 128 - 5 * col);
                    }
                    rectfill(world->buf, (x)*TILESIZE, (y)*TILESIZE, (x + 1) * TILESIZE - 1, (y + 1) * TILESIZE - 1, drawn_color);
                }
                
                if (ns_check_flags_at(world, x, y, TILE_IS_BLOOD_STAINED))
                {
                   for(int j = 0; j < 10; j++)
                   {
                       int x_pos = x * TILESIZE + HALFTILESIZE;
                       int y_pos = y * TILESIZE + HALFTILESIZE;
                       int dx = 3 - (x * 13 + y * 7 + j * 3 + world->current_room) % 7;
                       int dy = 3 - (j * 13 + x * 7 + y * 3 + world->current_room) % 7;
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
                    int wall_type = ns_getWallTypeAt(world, x, y);
                    if (wall_type)
                    {
                        if (wall_type == WALL_NORMAL || wall_type == WALL_PENTAGRAM)
                        {
                            rectfill(world->buf, x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 14, y * TILESIZE + lev + 14, col_wall);
                        }
                        if (wall_type == WALL_LAVA && (((int)abs(lava_fluctuations) / 5 + (y & x) * (y | x)) % 15 == lev || lev == 15))
                        {
                            rectfill(world->buf, x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 14, y * TILESIZE + lev + 14, makecol(colcalc, colcalc >> 1, colcalc >> 2));
                        }
                    }
                    if (lev == 0)
                    {
                        if (wall_type == WALL_PENTAGRAM)
                        {
                            masked_blit(world->spr, world->buf, 47, 145, x * TILESIZE - 15, y * TILESIZE - 15, 30, 30);
                        }
                        else if (ns_check_flags_at(world, x, y, TILE_IS_EXIT_POINT))
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

void draw_player_legend(World *world)
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

void move_and_draw_body_parts(World *world)
{
    for (int x = 0; x < ENEMYCOUNT; x++)
    {
        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            int bonesturn = 0;
            BodyPart *bodypart = &world->enm[x].bodyparts[j];
            if (bodypart->exists && world->enm[x].roomid == world->current_room)
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
                        if (get_wall_type_at(world, bodypart->x, bodypart->y))
                        {
                            bodypart->dx = -bodypart->dx;
                        }
                        bodypart->y += bodypart->dy;
                        if (get_wall_type_at(world, bodypart->x, bodypart->y))
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

int progress_and_draw_explosions(World *world)
{
    int vibrations = 0;
    for (int x = 0; x < EXPLOSIONCOUNT; x++)
    {
        Explosion *ex = &world->explosion[x];
        if (!ex->exists)
            continue;
        vibrations++;
        masked_blit(world->explos_spr, world->buf, (ex->sprite) * 64, (ex->phase / 10) * 64, ex->x - 32, ex->y - 32, 64, 64);
        ex->phase += 8;
        if (ex->phase >= 240)
        {
            ex->exists = 0;
        }
    }
    return vibrations;
}

void display_level_info(World *world, int mission, int mission_count, BITMAP *bmp_levclear, FONT *font)
{
    clear_to_color(world->buf, 0);
    blit(bmp_levclear, world->buf, 0, 0, 0, world->buf->h - bmp_levclear->h, bmp_levclear->w, bmp_levclear->h);
    textprintf_ex(world->buf, font, 5, 5, WHITE, -1, "Level cleared!");
    textprintf_ex(world->buf, font, 5, 30, WHITE, -1, "Now entering level %d / %d.", mission + 1, mission_count);
    textprintf_ex(world->buf, font, 5, 120, WHITE, -1, "Press enter to continue!");
    stretch_blit(world->buf, screen, 0, 0, 480, 360, 0, 0, screen->w, screen->h);
}

void show_gold_hint(World *world, int number)
{
  sprintf(world->hint.text, "- %d", number);
  world->hint.loc.x = world->plr.x - 15;
  world->hint.loc.y = world->plr.y - 30;
  world->hint.dim = 4;
  world->hint.time_shows = 60;
}
