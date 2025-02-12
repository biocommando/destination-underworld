#pragma once

#define NUM_BEST_TIMES 3

// Structure containing best times for one level and one game mode
struct best_times
{
    // Game mode flags
    int game_modifiers;
    // Mission number
    int mission;
    // Best times as seconds. Array ordered in ascending order.
    float times[NUM_BEST_TIMES];
};

/*
 * Reads best times structure from datadir\mission_pack\best_times.dat.
 * The best_times argument should have the game_modifiers and mission
 * fields initialized already, only the times array will be populated based
 * on the data game mode and mission number data.
 *
 * Best times file uses record file format. The syntax is:
 * MISSION=%d;MODE=%d;I=%d; %f
 *
 * where:
 * MISSION = best_times.mission
 * MODE = best_times.game_modifiers
 * I = the index in best_times.times array
 *
 * If record is not found, initializes array with value 1e10.
 */
int populate_best_times(const char *mission_pack, struct best_times *best_times);

/*
 * Save the new best time structure to the file. See populate_best_times for more information.
 */
int save_best_times(const char *mission_pack, struct best_times *best_times);

/*
 * Checks if the best time was beaten by the provided time. If that was the case,
 * modifies the best_times structure. Returns the index of the best time beaten.
 * Returns -1 if none of the times were beaten.
 */
int check_time_beaten(struct best_times *best_times, float time);