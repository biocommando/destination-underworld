#pragma once

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

/*
 * Makes the game loop wait until the start of next animation frame.
 * The wait is clock based so it should be pretty stable.
 */
void game_loop_rest(double *state);

/*
 * Get a random number in range [0, 1[.
 */
double random();

// Structure representing a 2D point
typedef struct coordinates
{
    double x;
    double y;
} Coordinates;

/*
 * Calculate the cost of next perk based on current perk status flags.
 */
int calculate_next_perk_xp(int perks);