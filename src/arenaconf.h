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

// Highscores for all arenas and all game modes.
// The arrays are indexed so that the first index corresponds to the
// arena index in ArenaConfigs and the second index is just a running number
// for different game modifier flag combinations (so that each game mode can
// have unique highscore).
typedef struct
{
    // Number of kills
    int kills[ARENACONF_MAX_NUMBER_OF_ARENAS][ARENACONF_HIGHSCORE_MAP_SIZE];
    // Game modifier flags
    int mode[ARENACONF_MAX_NUMBER_OF_ARENAS][ARENACONF_HIGHSCORE_MAP_SIZE];
    // Record file is only accessed if the entry is marked as "dirty"
    int dirty[ARENACONF_MAX_NUMBER_OF_ARENAS][ARENACONF_HIGHSCORE_MAP_SIZE];
} ArenaHighscore;

/*
 * Read the ArenaConfigs structure. Uses record file format.
 * Expects to find following types of records:
 * number_of_arenas amount
 *   Read this many ArenaConfigs from the file
 * arena_%d name level_number=%d
 *   Populates ArenaConfig structure. The underscores in the name parameter
 *   are substituted with spaces.
 */
void read_arena_configs(const char *filename, ArenaConfigs *config);

/*
 * Read highscore file. See access_arena_highscore.
 */
void read_arena_highscores(const char *filename, ArenaHighscore *highscore);

/*
 * Write highscore file. See access_arena_highscore.
 */
void write_arena_highscores(const char *filename, ArenaHighscore *highscore);

/*
 * Get the highscore for requested mission and game mode. If they don't exist
 * return 0.
 */
int get_arena_highscore(int mission, int game_mode);

/*
 * Set the highscore for requested mission and game mode.
 */
void set_arena_highscore(int mission, int game_mode, int score);