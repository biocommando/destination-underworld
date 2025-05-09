#pragma once

#include "world.h"

// Text that "flies" across the screen
struct fly_in_text
{
  int x;
  char text[64];
};
/*
 * Set the "fly-in" text and set the position to right screen edge.
 */
void set_fly_in_text(struct fly_in_text *fit, const char *text);
/*
 * Draws the "fly-in" text and progesses its state (moves it to left on screen).
 */
void draw_fly_in_text(struct fly_in_text *fly_in_text);
/*
 * Draw the enemy sprite. Takes care of walking animation.
 */
void draw_enemy(Enemy *enm, World *world);
/*
 * Draws a similar health bar as is used for player for the boss
 * enemy if it exists in the current room. The health bar has max. 6 hearts
 * but each heart represents one 6th of the original health.
 */
void draw_boss_health_bar(const World *world);
/*
 * Draws the legend that has the health meter, ammo counter / reload status indicator,
 * gold count and weapon mode.
 */
void draw_player_legend(World *world, int x, int y);
/*
 * Draw and progress the rune of protection powerup animation.
 */
void draw_rune_of_protection_indicator(World *world);
/*
 * Displays the fading crosshair that indicates the player's (new) direction and
 * progresses its state.
 */
void display_plr_dir_helper(const World *world, int *plr_dir_helper_intensity);
/*
 * Progress and apply the zoom-in transformation that happens when the player dies.
 */
void progress_player_death_animation(const World *world);
/*
 * Does the transformations used in the main game loop:
 * shakes the screen, scales it 3 times larger and possibly centers the play area
 * in full screen mode.
 */
void apply_game_screen_transform(int vibrations);
/*
 * Draws the map. Has two modes:
 * 1. draw_walls = 0:
 * Draws only floors and tiles that are on the same level as floor (e.g. level exit).
 * Shade map is subtracted directly from the floor color.
 * 2. draw_walls = 1:
 * Draws only walls (without shadows, they should be drawn before walls).
 *
 * The map needs to be drawn in stages because of the fake isometric perspective.
 *
 * Vibration intensity also affects the wall/floor color directly (explosions make the
 * level a bit brighter for a moment).
 */
void draw_map(World *world, int draw_walls, int vibration_intensity);
/*
 * Draws wall shadows as an semi-opaque layer. This would allow different items that are next to
 * the walls to be in the shadow but because the fake isometric view is just added on top of
 * everything and the enemy sprites are always front facing, the effect just looks weird and wrong.
 * So only the ting of the body parts is affected by the shadow.
 */
void draw_wall_shadows(World *world);
/*
 * Moves the body parts in the current room around. The parts are slowed down by a friction and they
 * bounce off walls.
 */
void move_and_draw_body_parts(World *world);
/*
 * Progresses and draws explosion effects (only visuals). Returns the vibration intensity based on
 * how many explosions are in progress at the moment.
 */
int progress_and_draw_explosions(World *world);
/*
 * Progresses and draws sparkles and sparkle circles.
 */
void progress_and_draw_sparkles(World *world);
/*
 * Progresses and draws flames.
 */
void progress_and_draw_flame_fx(World *world);
/*
 * Displays the level ending screen that contains e.g. story text and best times.
 */
void display_level_info(World *world, int mission, int mission_count, long completetime);
/*
 * Shows a hint text near player sprite that tells how much gold was used on a powerup.
 */
void show_gold_hint(World *world, int number);
/*
 * Draws the hint text and progresses its fading away.
 */
void draw_hint(World *world);
/*
 * Displays a level info screen that contains the mini map and some metadata on the level.
 */
void show_ingame_info_screen(World *world);
/*
 * Draws oval shadows under enemies. Not in use because it looks stupid because of the fake
 * isometric perspective.
 */
void draw_enemy_shadows(World *world);
void draw_uber_wizard_weapon_fx(World *world);
