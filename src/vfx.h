#pragma once

#include "world.h"

/*
 * Clears the sparkle FX and explosions.
 */
void clear_visual_fx(World *);
/*
 * Set the speed for all bodyparts to 0.
 */
void stop_bodyparts(World *);
/*
 * Create body parts for the dead enemy. Initializes them to move to a random direction.
 */
void spawn_body_parts(Enemy *enm);
/* Checks if there are so many stacked bodyparts on a single tile
 * that some excess parts should be removed. Checks one tile at a time
 * and proceeds automatically to the next tile on each call.*/
void cleanup_bodyparts(World *world);
/*
 * Creates one explosion with the center at position x, y.
 */
void create_explosion(int x, int y, World *world, double intensity);
/*
 * Creates one sparkle effect with a sparkle circle effect at x, y. The count parameter tells the
 * number of sparkles to create. The parameter color is a number from 0 to 2 or -1 for random color
 * for each sparkle. The parameter circle_duration will affect the sparkle circle effect duration only.
 */
void create_sparkles(int x, int y, int count, int color, int circle_duration, World *world);
/*
 * Creates a circular "crater" around the x, y center position, with the radius of spread,
 * where the closest tiles get the darkest. Modifies the World.floor_shade_map array.
 */
void create_shade_around_hit_point(int x, int y, int spread, World *world);

/*
 * Create a single flame ember
 */
void create_flame_fx_ember(int x, int y, struct flame_ember_fx *f);

void create_flame_fx(int x, int y, World *world);

void create_uber_wizard_weapon_fx(World *world, int x2, int y2, int type);