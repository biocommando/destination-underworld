#include <stdio.h>
#include <math.h>
#include "renderWorld.h"
#include "ducolors.h"
#include "predictableRandom.h"
#include "sprites.h"
#include "best_times.h"
#include "settings.h"
#include "helpers.h"
#include "game_playback.h"
#include "vfx.h"

static const double expl_sin_cos_table[2][10] = {
    {0.19866933079506122, 0.7367955455941375, 0.9934909047357762,
     0.8707065057822322, 0.4153418158455323, -0.19866933079506127,
     -0.7367955455941376, -0.9934909047357762, -0.8707065057822322,
     -0.4153418158455324},
    {0.9800665778412416, 0.6761156143683099, 0.11391146653120064,
     -0.49180298981248144, -0.9096654198166136, -0.9800665778412416,
     -0.6761156143683098, -0.11391146653120054, 0.49180298981248133,
     0.9096654198166136}};

void set_fly_in_text(struct fly_in_text *fit, const char *text)
{
    fit->x = SCREEN_W;
    strcpy(fit->text, text);
}

inline void draw_fly_in_text(struct fly_in_text *fly_in_text)
{
    int *x = &fly_in_text->x;
    if (*x > -400)
    {
        al_draw_textf(get_font(), WHITE, *x, 170, -1, fly_in_text->text);
        if (*x > SCREEN_W / 8 * 3 && *x < SCREEN_W / 8 * 5)
        {
            *x -= 4;
        }
        else
        {
            *x -= 10;
        }
    }
}

void draw_enemy(Enemy *enm, World *world)
{
    if (enm->sprite == 9)
        draw_sprite_centered(world->spr, SPRITE_ID_TURRET, enm->x, enm->y);
    else
        draw_sprite_animated_centered(world->spr, SPRITE_ID_ENEMY(enm->sprite), enm->x, enm->y, enm->anim > 20, 0);
}

inline void draw_boss_health_bar(const World *world)
{
    if (world->boss && world->boss->roomid == world->current_room)
    {
        for (int i = 0; i < 6; i++)
        {
            if (world->boss->health >= (world->boss_fight_config->health * (i + 1) / 6))
                draw_sprite(world->spr, SPRITE_ID_HEALTH, world->boss->x - 23, world->boss->y - 18 + 4 * i);
        }
    }
}

static void draw_enemy_shadow(Enemy *enm)
{
    al_draw_filled_ellipse(enm->x - 3, enm->y + 12, 15, 5, al_map_rgba(0, 0, 0, 60));
}

void draw_enemy_shadows(World *world)
{
    draw_enemy_shadow(&world->plr);
    for (int i = 0; i < ENEMYCOUNT; i++)
    {
        Enemy *enm = &world->enm[i];
        if (enm->alive && enm->roomid == world->current_room)
        {
            draw_enemy_shadow(enm);
        }
    }
}

void draw_wall_shadows(World *world)
{
    const int floor_base_col = 100;
    const int shadow_base_col = floor_base_col - 44;
    for (int y = 0; y < MAPMAX_Y; y++)
    {
        for (int x = 1; x < MAPMAX_X; x++)
        {
            int shadowcolm1 = shadow_base_col;
            int wall_type = ns_get_wall_type_at(world, x, y);
            if (wall_type == WALL_NORMAL || wall_type == WALL_PENTAGRAM)
            {
                const Tile *t = ns_get_tile_at(world, x - 1, y);
                if (t->is_floor)
                    al_draw_filled_rectangle((x - 1) * TILESIZE, (y)*TILESIZE, (x)*TILESIZE + 1, (y + 1) * TILESIZE, al_map_rgba(0, 0, 0, shadowcolm1));
            }
        }
    }
}

static inline int progress_lava_fluctuations()
{
    static int lava_fluctuations = 0;
    if (++lava_fluctuations == 150)
        lava_fluctuations = -149;
    return lava_fluctuations;
}

static const int floor_base_col = 100;
static const int shadow_base_col = floor_base_col - 44;

void draw_map_floors(World *world, int vibration_intensity)
{
    int lava_fluctuations = progress_lava_fluctuations();
    for (int y = 0; y < MAPMAX_Y; y++)
    {
        for (int x = 0; x < MAPMAX_X; x++)
        {
            int fshd = world->floor_shade_map[world->current_room - 1][x][y] * 5;
            int shadowcol = shadow_base_col + 5 * vibration_intensity - fshd;
            shadowcol = shadowcol < 0 ? 0 : shadowcol;
            int floorcol = floor_base_col + 5 * vibration_intensity - fshd;
            floorcol = floorcol < 0 ? 0 : floorcol;
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
                al_draw_filled_rectangle((x)*TILESIZE, (y)*TILESIZE, (x + 1) * TILESIZE, (y + 1) * TILESIZE, drawn_color);
                if (!tile->is_exit_level)
                {
                    double multiplier = 1.2 + expl_sin_cos_table[0][(int)(x * 0.66)] * 0.15;
                    ALLEGRO_COLOR pattern_color = GRAY(floorcol * multiplier);
                    al_draw_filled_rectangle((x)*TILESIZE, (y)*TILESIZE, x * TILESIZE + HALFTILESIZE, y * TILESIZE + HALFTILESIZE, pattern_color);
                    al_draw_filled_rectangle((x)*TILESIZE + HALFTILESIZE, (y)*TILESIZE + HALFTILESIZE, (x + 1) * TILESIZE, (y + 1) * TILESIZE, pattern_color);
                }
            }

            if (tile->is_blood_stained)
            {
                for (int j = 0; j < 10; j++)
                {
                    int x_pos = TO_PIXEL_COORDINATES(x);
                    int y_pos = TO_PIXEL_COORDINATES(y);
                    int dx = 3 - (x * 13 + y * 7 + j * 3 + world->current_room) % 7;
                    int dy = 3 - (j * 13 + x * 7 + y * 3 + world->current_room) % 7;
                    for (int i = 0; i < 4; i++)
                    {
                        x_pos += dx;
                        y_pos += dy;
                        al_draw_filled_rectangle(x_pos - 2, y_pos - 2, x_pos + 2 + 1, y_pos + 2 + 1, al_map_rgb(floorcol + 30 - i * 5, 0, 0));
                    }
                }
            }

            if (tile->is_exit_level)
            {
                for (int i = 0; i < 5; i++)
                {
                    double angle = (lava_fluctuations + i * 20) * ALLEGRO_PI / 50;
                    draw_sprite_animated_centered(world->spr, SPRITE_ID_SPARKLES,
                                                    TO_PIXEL_COORDINATES(x) + sin(angle) * 10,
                                                    TO_PIXEL_COORDINATES(y) + cos(angle) * 10, (rand() % 4), 0);
                }
            }
        }
    }
}

void draw_map_walls(World *world)
{
    int lava_fluctuations = progress_lava_fluctuations();
    for (int y = 0; y < MAPMAX_Y; y++)
    {
        for (int lev = 15; lev >= 0; lev--)
        {
            int colcalc = lev == 0 ? 165 : (15 - lev) * 10;
            colcalc += y * 10;
            colcalc = colcalc > 255 ? 255 : colcalc;
            ALLEGRO_COLOR col_wall = al_map_rgb(colcalc * world->map_wall_color[0],
                                                colcalc * world->map_wall_color[1],
                                                colcalc * world->map_wall_color[2]);

            for (int x = 0; x < MAPMAX_X; x++)
            {
                int wall_type = ns_get_wall_type_at(world, x, y);
                if (wall_type)
                {
                    if (wall_type == WALL_NORMAL || wall_type == WALL_PENTAGRAM)
                    {
                        al_draw_filled_rectangle(x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 15, y * TILESIZE + lev + 15, col_wall);
                    }
                    if (wall_type == WALL_LAVA && (((int)abs(lava_fluctuations) / 5 + (y & x) * (y | x)) % 15 == lev || lev == 15))
                    {
                        al_draw_filled_rectangle(x * TILESIZE - 15 + lev, y * TILESIZE - 15 + lev, x * TILESIZE + lev + 15, y * TILESIZE + lev + 15, al_map_rgb(colcalc, colcalc >> 1, colcalc >> 2));
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
                            al_draw_filled_rectangle(x * TILESIZE - 15, y * TILESIZE - 15, x * TILESIZE + 15, y * TILESIZE + 15, GRAY_A(100, 100));
                            al_draw_filled_rectangle(x * TILESIZE - 10, y * TILESIZE - 10, x * TILESIZE + 10, y * TILESIZE + 10, GRAY_A(100, 128));
                            al_draw_filled_rectangle(x * TILESIZE - 5, y * TILESIZE - 5, x * TILESIZE + 5, y * TILESIZE + 5, GRAY_A(80, 150));
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

        const char *weapon_text = world->plr.shots > 1 ? "L" : "S";
        if (*world->game_modifiers & GAMEMODIFIER_UBER_WIZARD)
        {
            if (world->plr.shots == 1)
                weapon_text = "Death ray";
            if (world->plr.shots == 2)
                weapon_text = "Blast";
            if (world->plr.shots == 3)
                weapon_text = "Leech";
            if (world->plr.shots == 4)
                weapon_text = "Shield";
    
            if (world->plr.reload)
                weapon_text = "Wait";
        }
        al_draw_text(get_font_tiny(), al_map_rgb(255, 240, 0), x - 21, y - 14 + 4 * world->plr.health, 0, weapon_text);
        int gold = world->plr.gold;
        if (gold > 99)
            gold = 99;
        al_draw_textf(get_font_tiny(), al_map_rgb(255, 240, 0), x - 25, y - 5 + 4 * world->plr.health, 0, "%d", gold);

        if (world->potion_duration > 0)
        {
            int sprite = 8 * world->potion_duration / (POTION_DURATION_CAP + 1);
            draw_sprite_animated(world->spr, SPRITE_ID_POTION_DURATION, x - 32, y - 18, sprite, 0);
            int yeffect = -1;
            if (world->potion_effect_flags & POTION_EFFECT_SHIELD_OF_FIRE)
                draw_sprite_animated(world->spr, SPRITE_ID_POTION_EFFECT, x - 32, (y - 8) + (++yeffect) * 6, 0, 0);
            if (world->potion_effect_flags & POTION_EFFECT_STOP_ENEMIES)
                draw_sprite_animated(world->spr, SPRITE_ID_POTION_EFFECT, x - 32, (y - 8) + (++yeffect) * 6, 1, 0);
            if (world->potion_effect_flags & POTION_EFFECT_FAST_PLAYER)
                draw_sprite_animated(world->spr, SPRITE_ID_POTION_EFFECT, x - 32, (y - 8) + (++yeffect) * 6, 2, 0);
            if (world->potion_effect_flags & POTION_EFFECT_BOOSTED_SHOTS)
                draw_sprite_animated(world->spr, SPRITE_ID_POTION_EFFECT, x - 32, (y - 8) + (++yeffect) * 6, 3, 0);
            if (world->potion_effect_flags & POTION_EFFECT_ALL_BULLETS_HURT_MONSTERS)
                draw_sprite_animated(world->spr, SPRITE_ID_POTION_EFFECT, x - 32, (y - 8) + (++yeffect) * 6, 4, 0);
            if (world->potion_effect_flags & POTION_EFFECT_HEALING)
                draw_sprite_animated(world->spr, SPRITE_ID_POTION_EFFECT, x - 32, (y - 8) + (++yeffect) * 6, 5, 0);
            if (world->potion_turbo_mode)
            {
                draw_sprite(world->spr, SPRITE_ID_CLUSTER, x, y - 32);
                draw_sprite(world->spr, SPRITE_ID_CLUSTER, x - 32, y);
                draw_sprite(world->spr, SPRITE_ID_CLUSTER, x, y + 32);
                draw_sprite(world->spr, SPRITE_ID_CLUSTER, x + 32, y);
            }
        }
    }
}

inline void draw_rune_of_protection_indicator(World *world)
{
    static int phase = 0;
    phase++;
    if (world->powerups.rune_of_protection_active)
    {
        if (world->powerups.rune_of_protection_active < 0)
        {
            world->powerups.rune_of_protection_active++;
            draw_sprite_centered(world->spr, SPRITE_ID_RUNE_OF_PROTECTION,
                                 world->plr.x + world->powerups.rune_of_protection_active * sin(phase * 0.15),
                                 world->plr.y + world->powerups.rune_of_protection_active * cos(phase * 0.15));
        }
        else
        {
            for (int i = 0; i < world->powerups.rune_of_protection_active; i++)
            {
                draw_sprite_centered(world->spr, SPRITE_ID_RUNE_OF_PROTECTION,
                                     world->plr.x - TILESIZE * sin(phase * 0.15 + i),
                                     world->plr.y - TILESIZE * cos(phase * 0.15 + i));
            }
        }
    }
}

inline void display_plr_dir_helper(const World *world, int *plr_dir_helper_intensity)
{
    if (*plr_dir_helper_intensity > 0)
    {
        int cx = world->plr.x + world->plr.dx * TILESIZE * 3 / 2;
        int cy = world->plr.y + world->plr.dy * TILESIZE * 3 / 2;
        ALLEGRO_COLOR col = al_map_rgb(2 * *plr_dir_helper_intensity, 0, 0);
        al_draw_circle(cx, cy, *plr_dir_helper_intensity * TILESIZE / 600, col, 1);
        if (*plr_dir_helper_intensity > PLR_DIR_HELPER_INITIAL_INTENSITY / 2)
        {
            int line_offs = *plr_dir_helper_intensity / 20;
            al_draw_line(cx - line_offs, cy, cx + line_offs, cy, col, 1);
            al_draw_line(cx, cy - line_offs, cx, cy + line_offs, col, 1);
        }
        *plr_dir_helper_intensity -= 3;
    }
}

void progress_player_death_animation(const World *world)
{
    int startx, starty;
    startx = world->plr.x - TILESIZE * 3 / 2 - world->plr.reload;
    if (startx < 0)
        startx = 0;
    starty = world->plr.y - TILESIZE - world->plr.reload * 0.75;
    if (starty < 0)
        starty = 0;
    double scale = 1 + (100 - world->plr.reload) / 20.0;
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_translate_transform(&transform, -startx, -starty);
    al_scale_transform(&transform, 3 * scale, 3 * scale);
    al_use_transform(&transform);
}

inline void apply_game_screen_transform(int vibrations)
{
    int offset_x = 2 * vibrations - rand() % (1 + 2 * vibrations);
    int offset_y = 2 * vibrations - rand() % (1 + 2 * vibrations);
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);

    al_translate_transform(&transform, offset_x, offset_y);
    al_scale_transform(&transform, 3, 3);
    al_use_transform(&transform);
}

void move_and_draw_body_parts(World *world)
{
    for (int i = 0; i < ENEMYCOUNT; i++)
    {
        if (world->enm[i].roomid != world->current_room || world->enm[i].health > 0)
        {
            continue;
        }

        for (int j = 0; j < BODYPARTCOUNT; j++)
        {
            int bonesturn = 0;
            BodyPart *bodypart = &world->enm[i].bodyparts[j];
            if (bodypart->exists)
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

static void draw_explosion_circle(double x, double y, double intensity, double radius)
{
    const int red = MIN((intensity * 0.5 + 0.5) * 255, 255);
    const double sqintens = intensity * intensity;
    const int green = MIN((sqintens * 0.8 + 0.2) * 255, 255);
    const int blue = MIN((sqintens * sqintens * sqintens * 0.8) * 255, 255);

    const ALLEGRO_COLOR col = al_map_rgb(red, green, blue);

    al_draw_filled_circle(x, y, radius, col);
}

void progress_and_draw_flame_fx(World *world)
{
    static int flame_phase = 0;
    flame_phase++;
    for (int i = 0; i < FLAME_FX_COUNT; i++)
    {
        struct flame_fx *f = &world->flames[i];
        if (f->duration == 0)
            continue;
        if (f->duration > 1)
            f->duration--;
        int num_circles_left = EMBERS_PER_FLAME_FX;
        for (int j = 0; j < EMBERS_PER_FLAME_FX; j++)
        {
            struct flame_ember_fx *fc = &f->embers[j];
            if (fc->r == 0)
            {
                num_circles_left--;
                continue;
            }
            al_draw_filled_rectangle(fc->loc.x - fc->r / 3, fc->loc.y - fc->r / 3, fc->loc.x + fc->r / 3, fc->loc.y + fc->r / 3, fc->color);
            fc->loc.x += expl_sin_cos_table[0][(flame_phase + i + j) % 10] * fc->speed / 2;
            fc->loc.y -= fc->speed * 2;
            fc->r--;
            fc->color.r *= 0.9f;
            fc->color.g *= 0.8f;
            if (fc->r == 0 && f->duration > j * 10 + 1)
            {
                create_flame_fx_ember(f->loc.x, f->loc.y, fc);
            }
        }
        if (!num_circles_left)
            f->duration = 0;
    }
}

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

            draw_explosion_circle(c->loc.x + ex->x, c->loc.y + ex->y, c->i * .9, c->r);
            draw_explosion_circle(c->loc.x + ex->x, c->loc.y + ex->y, c->i, c->r * .8);
            draw_explosion_circle(c->loc.x + ex->x, c->loc.y + ex->y, c->i * 1.1, c->r * .7);

            // Fade

            double move_speed = random() * c->i * 5;

            c->loc.x += c->loc.x > circle_max_radius / 2 ? move_speed : -move_speed;
            c->loc.y += c->loc.y > circle_max_radius / 2 ? move_speed : -move_speed;
            double multiplier_factor = log(sqrt(j) + 2) / 10;
            double intensity_multiplier = (1 - multiplier_factor) + random() * multiplier_factor;
            c->i *= intensity_multiplier;
            c->r *= intensity_multiplier;
        }

        if (ex->phase < 8)
        {
            for (int j = 0; j < 10; j++)
            {
                double dx = expl_sin_cos_table[0][j];
                double dy = expl_sin_cos_table[1][j];

                for (int k = 4; k < 6; k += 2)
                {
                    draw_sprite_animated_centered(world->spr, SPRITE_ID_BULLET,
                                                  dx * ex->phase * k * ex->intensity + ex->x,
                                                  dy * ex->phase * k * ex->intensity + ex->y,
                                                  (ex->phase + j) % 4,
                                                  -1 - ex->phase / 2);
                }
            }
        }
        if (ex->emit_blast_wave && ex->phase < 12)
            al_draw_filled_circle(ex->x, ex->y, ex->phase * 7, al_map_rgba(0, 0, 0, (12 - ex->phase) * 6.6));

        if (++ex->phase >= 30)
        {
            ex->exists = 0;
        }
    }
    return vibrations;
}

void progress_and_draw_sparkles(World *world)
{
    for (int i = 0; i < SPARKLE_FX_CIRCLE_COUNT; i++)
    {
        struct sparkle_fx_circle *fxc = &world->sparkle_fx_circle[i];
        if (fxc->duration > 0)
        {
            al_draw_circle(fxc->loc.x, fxc->loc.y, fxc->time * 5, fxc->color, fxc->duration / 4);
            al_draw_circle(fxc->loc.x, fxc->loc.y, fxc->time * 3, fxc->color, fxc->duration / 5);
            al_draw_circle(fxc->loc.x, fxc->loc.y, fxc->time * 1, fxc->color, 1);
            fxc->duration--;
            fxc->time++;
        }
    }
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

static inline void check_valid_time_for_display(float *f)
{
    *f = *f > 9999.9f ? 9999.9f : *f;
}

void display_level_info(World *world, int mission, int mission_count, long completetime)
{
    const int y_margin = al_get_font_line_height(get_menu_font()) + 2;
    const int y_margin_small_text = al_get_font_line_height(get_font()) + 1;
    al_clear_to_color(BLACK);
    int y = 5;
    al_draw_text(get_menu_font(), GRAY(200), 5, y, 0, world->mission_display_name);
    y += y_margin;
    struct best_times best_times;
    best_times.game_modifiers = *world->game_modifiers;
    best_times.mission = mission;
    populate_best_times(get_game_settings()->mission_pack, &best_times);
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
    const int *record_mode = get_playback_mode();
    if (beat_idx >= 0 && *record_mode != RECORD_MODE_PLAYBACK)
    {
        save_best_times(get_game_settings()->mission_pack, &best_times);
    }
    y += y_margin_small_text;
    if (mission < mission_count)
        al_draw_textf(get_font(), GRAY(200), 5, y, 0, "Now entering level %d / %d.", mission + 1, mission_count);
    y += y_margin_small_text + 5;
    al_draw_filled_rectangle(0, y - 3, SCREEN_W + 1, y + y_margin_small_text * world->story_after_mission_lines + 2 + 1, GRAY(20));
    for (int i = 0; i < world->story_after_mission_lines; i++)
    {
        al_draw_textf(get_font(), GRAY(140), 5, y, 0, "%s", world->story_after_mission[i]);
        y += y_margin_small_text;
    }
    y += 2;
    if (world->custom_story_image[0])
    {
        ALLEGRO_BITMAP *img = al_load_bitmap(world->custom_story_image);
        al_draw_scaled_bitmap(img, 0, 0, al_get_bitmap_width(img), al_get_bitmap_height(img), 0, y, SCREEN_W, SCREEN_H - y, 0);
        al_destroy_bitmap(img);
    }
    else
    {
        al_draw_scaled_bitmap(world->spr, 105, 0, 160, 100, 0, y, SCREEN_W, SCREEN_H - y, 0);
    }
    al_draw_textf(get_menu_font(), GRAY(200), 5, SCREEN_H - y_margin, 0, "Press enter to continue!");
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

inline void draw_hint(World *world)
{
    if (world->hint.time_shows > 0)
    {
        world->hint.time_shows--;
        int hint_col = world->hint.time_shows * world->hint.dim;
        al_draw_textf(get_font(), GRAY(hint_col), world->hint.loc.x, world->hint.loc.y, -1, world->hint.text);
    }
}

void show_ingame_info_screen(World *world)
{
    wait_key_release(ALLEGRO_KEY_M);
    al_clear_to_color(BLACK);
    const int map_tile_size = 8;
    const int max_minimaps_per_row = SCREEN_W / (map_tile_size * (MAPMAX_X + 1));
    for (int i = 0; i < ROOMCOUNT; i++)
    {
        const int offset_x = map_tile_size + map_tile_size * (MAPMAX_X + 1) * (i % max_minimaps_per_row);
        const int offset_y = map_tile_size + map_tile_size * (MAPMAX_Y + 1) * (i / max_minimaps_per_row);
        for (int x = 0; x < MAPMAX_X; x++)
        {
            for (int y = 0; y < MAPMAX_Y; y++)
            {
                int xx = offset_x + x * map_tile_size;
                int yy = offset_y + y * map_tile_size;
                Tile *t = &world->map[i][x][y];
                ALLEGRO_COLOR color;
                if (t->is_floor)
                {
                    color = GRAY(127);
                }
                else if (t->is_exit_level)
                {
                    color = BLUE;
                }
                else
                {
                    color = RED;
                }
                al_draw_filled_rectangle(xx, yy, xx + map_tile_size, yy + map_tile_size, color);
                if (world->map[i][x][y].is_exit_point)
                {
                    al_draw_textf(get_font_tiny(), GRAY(200), xx, yy, 0, "%d", world->map[i][x][y].data);
                }
            }
        }
        if (world->plr.roomid == i + 1)
        {
            al_draw_filled_circle(offset_x + world->plr.x / TILESIZE * map_tile_size + map_tile_size / 2,
                                  offset_y + world->plr.y / TILESIZE * map_tile_size + map_tile_size / 2, map_tile_size / 2, WHITE);
        }
        int num_enemies = 0;
        for (int e = 0; e < ENEMYCOUNT; e++)
        {
            Enemy *enm = &world->enm[e];
            if (enm->alive && enm->roomid == i + 1 && enm != world->boss)
                num_enemies++;
        }
        al_draw_textf(get_font_tiny(), GRAY(200), offset_x, offset_y - map_tile_size, 0, "#%d - Enemies: %d", i + 1,
                      num_enemies);
        for (int e = 0; e < world->boss_fight_configs[i].num_events; e++)
        {
            BossFightEventConfig *evt = &world->boss_fight_configs[i].events[e];
            if (evt->event_type == BFCONF_EVENT_TYPE_MODIFY_TERRAIN)
            {
                int xx = offset_x + evt->parameters[0] * map_tile_size;
                int yy = offset_y + evt->parameters[1] * map_tile_size;
                al_draw_textf(get_font_tiny(), GRAY(200), xx, yy, 0, "S");
            }
            else if (evt->event_type == BFCONF_EVENT_TYPE_SPAWN)
            {
                int xx = offset_x + evt->spawn_point.x * map_tile_size;
                int yy = offset_y + evt->spawn_point.y * map_tile_size;
                al_draw_textf(get_font_tiny(), GRAY(200), xx, yy, 0, "E");
            }
            else if (evt->event_type == BFCONF_EVENT_TYPE_SPAWN_POTION)
            {
                int xx = offset_x + evt->parameters[0] * map_tile_size;
                int yy = offset_y + evt->parameters[1] * map_tile_size;
                al_draw_textf(get_font_tiny(), GRAY(200), xx, yy, 0, "P");
            }
        }
    }
    int offset_x = map_tile_size + map_tile_size * (MAPMAX_X + 1) * (max_minimaps_per_row - 1);
    const int y_incr = 12;
    int offset_y = map_tile_size + map_tile_size * (MAPMAX_Y + 1) * (max_minimaps_per_row - 1);

    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "Legend:");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- Gray: floor");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- Red: wall");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- Blue: exit level");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- White circle: YOU");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- 1-8: room entrance");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- S: tile may be swapped");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- E: enemy spawn point");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "- P: potion spawn point");
    offset_y += y_incr;
    al_draw_textf(get_font(), GRAY(200), offset_x, offset_y, 0, "(press space to continue)");

    offset_y = map_tile_size * (MAPMAX_Y + 1) * max_minimaps_per_row;
    offset_x = map_tile_size;
    al_draw_textf(get_font_tiny(), GRAY(200), offset_x, offset_y, 0, "Enemy counts:          Kills: %d", world->kills);
    offset_y += map_tile_size;

    for (int et = 0; et < ENEMY_TYPE_COUNT; et++)
    {
        int num_enemies = 0;
        for (int e = 0; e < ENEMYCOUNT; e++)
        {
            Enemy *enm = &world->enm[e];
            if (enm->alive && enm->sprite == et)
                num_enemies++;
        }
        draw_sprite_animated(world->spr, SPRITE_ID_ENEMY(et), offset_x, offset_y, 0, 0);
        offset_x += TILESIZE;
        al_draw_textf(get_font_tiny(), GRAY(200), offset_x, offset_y + HALFTILESIZE, 0, "%d", num_enemies);
        offset_x += TILESIZE;
    }

    al_flip_display();

    const int keys[] = {ALLEGRO_KEY_SPACE, ALLEGRO_KEY_M};
    wait_key_presses(keys, 2);
}

void draw_uber_wizard_weapon_fx(World *world)
{
    UberWizardWeaponFx *fx = &world->uber_wizard_weapon_fx;
    if (fx->dim == 0)
        return;
    al_draw_line(fx->start.x, fx->start.y, fx->end.x, fx->end.y, al_map_rgb(fx->type == 0 ? fx->dim * 10 : 0, 0, fx->type == 1 ? fx->dim * 10 : 0), fx->dim / 4 + 1);
    fx->dim--;
}