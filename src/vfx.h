#pragma once

#include "world.h"

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