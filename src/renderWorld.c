#include <stdio.h>
#include <math.h>
#include "renderWorld.h"
#include "ducolors.h"
#include "predictableRandom.h"
#include "sprites.h"
#include "best_times.h"
#include "settings.h"

void draw_enemy(Enemy *enm, World *world)
{
    if (enm->sprite == 9)
        draw_sprite_centered(world->spr, SPRITE_ID_TURRET, enm->x, enm->y);
    else
        draw_sprite_animated_centered(world->spr, SPRITE_ID_ENEMY, enm->x, enm->y, enm->anim > 20, enm->sprite);
}

void draw_map(World *world, int draw_walls, int vibration_intensity)
{
    const int floor_base_col = 100; // 66
    const int shadow_base_col = floor_base_col - 44;

    static int lava_fluctuations = 0;
    if (++lava_fluctuations == 150)
        lava_fluctuations = -149;
    if (!draw_walls)
    {
        for (int y = 0; y < 12; y++)
        {
            for (int x = 0; x < 16; x++)
            {
                int fshd = world->floor_shade_map[world->current_room - 1][x][y] * 5;
                int shadowcol = shadow_base_col + 5 * vibration_intensity - fshd;
                shadowcol = shadowcol < 0 ? 0 : shadowcol;
                int floorcol = floor_base_col + 5 * vibration_intensity - fshd;
                floorcol = floorcol < 0 ? 0 : floorcol;
                int shadowcolm1 = 0;
                if (x > 0)
                {
                    shadowcolm1 = shadow_base_col + 5 * vibration_intensity - world->floor_shade_map[world->current_room - 1][x - 1][y] * 5;
                }
                if (shadowcolm1 < 0)
                {
                    shadowcolm1 = 0;
                }
                int wall_type = ns_get_wall_type_at(world, x, y);
                if (wall_type == WALL_NORMAL || wall_type == WALL_PENTAGRAM)
                {
                    if (x > 0 && !ns_get_tile_at(world, x - 1, y)->is_exit_level)
                        rectfill((x - 1) * TILESIZE, (y)*TILESIZE, (x)*TILESIZE, (y + 1) * TILESIZE - 1, GRAY(shadowcolm1));
                }
                Tile *tile = ns_get_tile_at(world, x, y);
                if (tile->is_floor || tile->is_exit_point || tile->is_exit_level)
                {
                    ALLEGRO_COLOR drawn_color = GRAY(floorcol);

                    if (tile->is_exit_point)
                    {
                        drawn_color = GRAY(shadowcol);
                    }
                    if (tile->is_exit_level)
                    {
                        drawn_color = al_map_rgb(0, 0, abs(lava_fluctuations) + 100);
                    }
                    rectfill((x)*TILESIZE, (y)*TILESIZE, (x + 1) * TILESIZE - 1, (y + 1) * TILESIZE - 1, drawn_color);
                }

                if (tile->is_blood_stained)
                {
                    for (int j = 0; j < 10; j++)
                    {
                        int x_pos = x * TILESIZE + HALFTILESIZE;
                        int y_pos = y * TILESIZE + HALFTILESIZE;
                        int dx = 3 - (x * 13 + y * 7 + j * 3 + world->current_room) % 7;
                        int dy = 3 - (j * 13 + x * 7 + y * 3 + world->current_room) % 7;
                        for (int i = 0; i < 4; i++)
                        {
                            x_pos += dx;
                            y_pos += dy;
                            rectfill(x_pos - 2, y_pos - 2, x_pos + 2, y_pos + 2, al_map_rgb(floorcol + 30 - i * 5, 0, 0));
                        }
                    }
                }

                if (tile->is_exit_level)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        double angle = (lava_fluctuations + i * 20) * AL_PI / 50;
                        draw_sprite_animated_centered(world->spr, SPRITE_ID_SPARKLES,
                            x * TILESIZE + HALFTILESIZE + sin(angle) * 10,
                            y * TILESIZE + HALFTILESIZE + cos(angle) * 10, (rand() % 4), 0);
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
                colcalc = colcalc > 255 ? 255 : colcalc;
                ALLEGRO_COLOR col_wall = al_map_rgb(colcalc * world->map_wall_color[0],
                                                 colcalc * world->map_wall_color[1],
                                                 colcalc * world->map_wall_color[2]);

                for (int x = 0; x < 16; x++)
                {
                    int wall_type = ns_get_wall_type_at(world, x, y);
                    if (wall_type)
                    {
                        if (wall_type == WALL_NORMAL || wall_type == WALL_PENTAGRAM)
                        {
                            rectfill(x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 14, y * TILESIZE + lev + 14, col_wall);
                        }
                        if (wall_type == WALL_LAVA && (((int)abs(lava_fluctuations) / 5 + (y & x) * (y | x)) % 15 == lev || lev == 15))
                        {
                            rectfill(x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 14, y * TILESIZE + lev + 14, al_map_rgb(colcalc, colcalc >> 1, colcalc >> 2));
                        }
                    }
                    if (lev == 0)
                    {
                        if (wall_type == WALL_PENTAGRAM)
                        {
                            draw_sprite_centered(world->spr, SPRITE_ID_PENTAGRAM_WALL, x * TILESIZE, y * TILESIZE);
                        }
                        else
                        {
                            Tile *tile = ns_get_tile_at(world, x, y);
                            if (tile->is_exit_point)
                            {
                                rectfill(x * TILESIZE - 15, y * TILESIZE - 15, x * TILESIZE + 14, y * TILESIZE + 14, GRAY(100));
                                rectfill(x * TILESIZE - 10, y * TILESIZE - 10, x * TILESIZE + 9, y * TILESIZE + 9, GRAY(90));
                                rectfill(x * TILESIZE - 5, y * TILESIZE - 5, x * TILESIZE + 4, y * TILESIZE + 4, GRAY(80));
                            }
                            if (tile->durability > 0)
                            {
                                draw_sprite_centered(world->spr, SPRITE_ID_CRACKED_WALL, x * TILESIZE, y * TILESIZE);
                            }
                        }
                    }
                }
            }
        }
    }
}

void draw_player_legend(World *world, int x, int y)
{
    if (world->plr.health > 0)
    {
        for (int n = 0; n < world->plr.health; n++)
            draw_sprite(world->spr, SPRITE_ID_HEALTH, x - 23, y - 18 + 4 * n);
        for (int n = 0; n < world->plr.ammo; n++)
        {
            draw_sprite_animated(world->spr, SPRITE_ID_AMMO, x + 17, y - 18 + 2 * n, world->plr.reload != 0, 0);
        }

        // 'S' or 'L'
        draw_sprite_animated(world->spr, SPRITE_ID_PLR_LEGEND_FONT, x - 21, y - 14 + 4 * world->plr.health, 10 + (world->plr.shots > 1), 0);
        int gold = world->plr.gold;
        if (gold > 99)
            gold = 99;
        if (gold > 9)
        {
            draw_sprite_animated(world->spr, SPRITE_ID_PLR_LEGEND_FONT, x - 25, y - 5 + 4 * world->plr.health, gold / 10, 0);
        }
        draw_sprite_animated(world->spr, SPRITE_ID_PLR_LEGEND_FONT, x - 21, y - 5 + 4 * world->plr.health, gold % 10, 0);

        if (world->potion_duration > 0)
        {
            int sprite = 8 * world->potion_duration / (POTION_DURATION_CAP + 1);
            draw_sprite_animated(world->spr, SPRITE_ID_POTION_DURATION, x - 32, y - 18, sprite, 0);
        }
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
                if (bodypart->velocity > 0.7)
                {
                    double bp_orig_x = bodypart->x;
                    double bp_orig_y = bodypart->y;
                    for (int travel_amt = bodypart->velocity; travel_amt > 0; travel_amt--)
                    {
                        int initially_inside_wall = get_wall_type_at(world, bodypart->x, bodypart->y);

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
                        if (initially_inside_wall && get_wall_type_at(world, bodypart->x, bodypart->y))
                            bodypart->exists = 0;
                    }

                    const double friction = 0.94 - (j + rand() % 5) * 0.003;

                    bodypart->velocity *= friction;
                    bonesturn = bodypart->velocity > 1;

                    ALLEGRO_COLOR blood_col = al_map_rgb(170 + rand() % 20, 10 + rand() % 8, 17 + rand() % 10);
                    al_draw_line(bp_orig_x, bp_orig_y, bodypart->x, bodypart->y, blood_col, 3);
                }

                if (bodypart->anim == 4)
                {
                    bodypart->anim = 0;
                }
                else if (bonesturn)
                {
                    bodypart->anim++;
                }

                draw_sprite_animated_centered(world->spr, SPRITE_ID_BODY_PART, (int)bodypart->x, (int)bodypart->y,
                    bodypart->anim > 1, bodypart->type - 1);
            }
        }
    }
}

void draw_explosion_circle(World *world, double x, double y, double intensity, double radius)
{
    const int red = MIN((intensity * 0.5 + 0.5) * 255, 255);
    const double sqintens = intensity * intensity;
    const int green = MIN((sqintens * 0.8 + 0.2) * 255, 255);
    const int blue = MIN((sqintens * sqintens * sqintens * 0.8) * 255, 255);

    const ALLEGRO_COLOR col = al_map_rgb(red, green, blue);

    al_draw_filled_circle(x - TILESIZE, y - TILESIZE, radius, col);
}

extern double random();

int progress_and_draw_explosions(World *world)
{
    const double circle_max_radius = 17;
    int vibrations = 0;
    for (int i = 0; i < EXPLOSIONCOUNT; i++)
    {
        Explosion *ex = &world->explosion[i];
        if (!ex->exists)
            continue;
        vibrations++;

        for (int j = 0; j < ex->circle_count; j++)
        {
            struct explosion_circle *c = &ex->circles[j];

            while (c->loc.x + c->r > circle_max_radius * 2 - 1)
                c->loc.x -= 1;
            while (c->loc.x - c->r < 0)
                c->loc.x += 1;
            while (c->loc.y + c->r > circle_max_radius * 2 - 1)
                c->loc.y -= 1;
            while (c->loc.y - c->r < 0)
                c->loc.y += 1;

            draw_explosion_circle(world, c->loc.x + ex->x, c->loc.y + ex->y, c->i * .9, c->r);
            draw_explosion_circle(world, c->loc.x + ex->x, c->loc.y + ex->y, c->i, c->r * .8);
            draw_explosion_circle(world, c->loc.x + ex->x, c->loc.y + ex->y, c->i * 1.1, c->r * .7);

            // Fade

            c->loc.x = c->loc.x > circle_max_radius / 2 ? c->loc.x + random() * 3 : c->loc.x - random() * 3;
            c->loc.y = c->loc.y > circle_max_radius / 2 ? c->loc.y + random() * 3 : c->loc.y - random() * 3;
            double multiplier_factor = log(sqrt(j) + 2) / 10;
            double intensity_multiplier = (1 - multiplier_factor) + random() * multiplier_factor;
            c->i *= intensity_multiplier;
            c->r *= intensity_multiplier;
        }

        if (++ex->phase >= 30)
        {
            ex->exists = 0;
        }
    }
    return vibrations;
}

void progress_and_draw_sparkles(World *world)
{
    for (int i = 0; i < SPARKLE_FX_COUNT; i++)
    {
        struct sparkle_fx *fx = &world->sparkle_fx[i];
        if (fx->duration > 0)
        {
            int color = fx->color;
            if (color == -1)
            {
                color = rand() % 3;
            }
            int trail_sprite = (fx->sprite + 1) % 4;
            draw_sprite_animated_centered(world->spr, SPRITE_ID_SPARKLES, fx->loc.x, fx->loc.y, trail_sprite, color);
            fx->loc.x += fx->dir.x;
            fx->loc.y += fx->dir.y;
            draw_sprite_animated_centered(world->spr, SPRITE_ID_SPARKLES, fx->loc.x, fx->loc.y, fx->sprite, color);
            fx->duration--;
            if (fx->duration % 3 == 0)
            {
                fx->sprite = trail_sprite;
            }
        }
    }
}

extern GameSettings game_settings;

static inline void check_valid_time_for_display(float *f)
{
    *f = *f > 9999.9f ? 9999.9f : *f;
}

void display_level_info(World *world, int mission, int mission_count, long completetime)
{
    al_clear_to_color(BLACK);
    al_draw_scaled_bitmap(world->spr, 100, 0, 214, 107, 0, SCREEN_H - 107 * 2, 214 * 2, 107 * 2, 0);
    int y = 5;
    al_draw_textf(get_font(), GRAY(200), 5, y, 0, "Level '%s' cleared!", world->mission_display_name);
    y += 15;
    struct best_times best_times;
    best_times.game_modifiers = world->game_modifiers;
    best_times.mission = mission;
    populate_best_times(game_settings.mission_pack, &best_times);
    float time_secs = (float)completetime / 40;
    int beat_idx = check_time_beaten(&best_times, (float)time_secs);
    
    check_valid_time_for_display(&time_secs);
    for (int i = 0; i < 3; i++)
        check_valid_time_for_display(best_times.times + i);
    float time0 = best_times.times[0];
    float time1 = best_times.times[1];
    float time2 = best_times.times[2];

    al_draw_textf(get_font(), GRAY(200), 5, y, 0, "Time: %.1f secs. Best times: %s%.1f, %s%.1f, %s%.1f",
                  time_secs, beat_idx == 0 ? "*" : "", time0,
                  beat_idx == 1 ? "*" : "", time1,
                  beat_idx == 2 ? "*" : "", time2);
    if (beat_idx >= 0)
    {
        save_best_times(game_settings.mission_pack, &best_times);
    }
    y += 15;
    if (mission < mission_count)
        al_draw_textf(get_font(), GRAY(200), 5, y, 0, "Now entering level %d / %d.", mission + 1, mission_count);
    y += 15;
    rectfill(0, y - 5, SCREEN_W, y + 15 * world->story_after_mission_lines - 5, GRAY(20));
    for (int i = 0; i < world->story_after_mission_lines; i++)
    {
        al_draw_textf(get_font(), GRAY(140), 5, y, 0, "%s", world->story_after_mission[i]);
        y += 15;
    }
    al_draw_textf(get_font(), GRAY(200), 5, SCREEN_H - 15, 0, "Press enter to continue!");
    // stretch_blit(world->buf, screen, 0, 0, 480, 360, 0, 0, screen->w, screen->h);
    al_flip_display();
}

void show_gold_hint(World *world, int number)
{
    sprintf(world->hint.text, "- %d", number);
    world->hint.loc.x = world->plr.x - 15;
    world->hint.loc.y = world->plr.y - 30;
    world->hint.dim = 4;
    world->hint.time_shows = 60;
}
