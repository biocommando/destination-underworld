#pragma once

#include "world.h"

/*
 * Draw the enemy sprite. Takes care of walking animation.
 */
void draw_enemy(Enemy *enm, World *world);
/*
 * Draws the legend that has the health meter, ammo counter / reload status indicator,
 * gold count and weapon mode.
 */
void draw_player_legend(World *world, int x, int y);
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
 * Displays a level info screen that contains the mini map and some metadata on the level.
 */
void show_ingame_info_screen(World *world);
/*
 * Draws oval shadows under enemies. Not in use because it looks stupid because of the fake
 * isometric perspective.
 */
void draw_enemy_shadows(World *world);
