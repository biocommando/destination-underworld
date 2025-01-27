#pragma once

#include <time.h>

/*
 * Makes the game loop wait until the start of next animation frame.
 * The wait is clock based so it should be pretty stable.
 */
void game_loop_rest(clock_t *state);

// Structure representing a 2D point
typedef struct coordinates
{
    double x;
    double y;
} Coordinates;
