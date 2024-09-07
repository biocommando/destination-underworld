#ifndef DU_ARENA_CONFIG
#define DU_ARENA_CONFIG

#include <stdio.h>

#define ARENACONF_MAX_NUMBER_OF_ARENAS 16
#define ARENACONF_HIGHSCORE_MAP_SIZE 10

typedef struct
{
    char name[64];
    int level_number;
} ArenaConfig;

typedef struct
{
    int number_of_arenas;
    ArenaConfig arenas[ARENACONF_MAX_NUMBER_OF_ARENAS];
} ArenaConfigs;

typedef struct
{
    int kills[ARENACONF_MAX_NUMBER_OF_ARENAS][ARENACONF_HIGHSCORE_MAP_SIZE];
    int mode[ARENACONF_MAX_NUMBER_OF_ARENAS][ARENACONF_HIGHSCORE_MAP_SIZE];
    int dirty[ARENACONF_MAX_NUMBER_OF_ARENAS][ARENACONF_HIGHSCORE_MAP_SIZE];
} ArenaHighscore;

void read_arena_configs(const char *filename, ArenaConfigs *config);

void read_arena_highscores(const char *filename, ArenaHighscore *highscore);

void write_arena_highscores(const char *filename, ArenaHighscore *highscore);

#endif
