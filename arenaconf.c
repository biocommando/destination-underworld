#include "arenaconf.h"

#include "iniRead.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>


void read_arena_configs(FILE *f, ArenaConfigs *config)
{
    int num = ini_read_int_value(f, "arenas", "number_of_arenas");

    if (num > ARENACONF_MAX_NUMBER_OF_ARENAS)
    {
        LOG("Invalid number of arenas %d\n", num);
        num = ARENACONF_MAX_NUMBER_OF_ARENAS;
    }
    config->number_of_arenas = num;
    
    for (int i = 0; i < num; i++)
    {
        char arena_segment[100];
        sprintf(arena_segment, "arena_%d", i);
        config->arenas[i].level_number = ini_read_int_value(f, arena_segment, "level_number");
        ini_read_string_value(f, arena_segment, "name", config->arenas[i].name);
        LOG("Read arena config: '%s' = %d\n", config->arenas[i].name, config->arenas[i].level_number);
    }
}

void read_arena_highscores(FILE *f, ArenaHighscore *highscore)
{
    for (int i = 0; i < ARENACONF_MAX_NUMBER_OF_ARENAS; i++)
    {
        for (int mi = 0; mi < ARENACONF_HIGHSCORE_MAP_SIZE; mi++)
        {
            char key[100];
            sprintf(key, "arena_%d_item_%d_mode", i, mi);
            highscore->mode[i][mi] = ini_read_int_value(f, "highscores", key);
            sprintf(key, "arena_%d_item_%d_kills", i, mi);
            highscore->kills[i][mi] = ini_read_int_value(f, "highscores", key);
        }
    }
}

void write_arena_highscores(FILE *f, ArenaHighscore *highscore)
{
    fprintf(f, "[highscores]\n");
    for (int i = 0; i < ARENACONF_MAX_NUMBER_OF_ARENAS; i++)
    {
        for (int mi = 0; mi < ARENACONF_HIGHSCORE_MAP_SIZE; mi++)
        {
            fprintf(f, "arena_%d_item_%d_mode=%d\n", i, mi, highscore->mode[i][mi]);
            fprintf(f, "arena_%d_item_%d_kills=%d\n", i, mi, highscore->kills[i][mi]);
        }
    }
}