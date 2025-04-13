#pragma once

#include <stdio.h>

#define ARENACONF_MAX_NUMBER_OF_ARENAS 16
#define ARENACONF_HIGHSCORE_MAP_SIZE 10

// Structure containing the meta data for an arena fight
typedef struct
{
    // The name of the arena
    char name[64];
    // The mission number that should be loaded for the arena fight
    int level_number;
} ArenaConfig;

// All arena fights available.
typedef struct
{
    // Number of initialized elements in arenas array
    int number_of_arenas;
    ArenaConfig arenas[ARENACONF_MAX_NUMBER_OF_ARENAS];
} ArenaConfigs;

/*
 * Read the ArenaConfigs structure. Uses record file format.
 * Expects to find following types of records:
 * number_of_arenas amount
 *   Read this many ArenaConfigs from the file
 * arena_%d level_number=%d name=%s
 *   Populates ArenaConfig structure. The name can have spaces etc.
 */
void read_arena_configs(const char *filename, ArenaConfigs *config);

/*
 * Get the highscore for requested mission and game mode. If they don't exist
 * return 0.
 *
 * Uses record file format. The records have the following format:
 * level_number=%d;mode=%d; %d
 * Where the placeholders are:
 * - level number that corresponds to the level_number field read in read_arena_configs.
 * - the game modifiers flags
 * - the highscore (number of kills)
 */
int get_arena_highscore(int mission, int game_mode);

/*
 * Set the highscore for requested mission and game mode.
 */
void set_arena_highscore(int mission, int game_mode, int score);