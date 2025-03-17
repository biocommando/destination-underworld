#include "arenaconf.h"

#include "record_file.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

void read_arena_configs(const char *filename, ArenaConfigs *config)
{
    int num = 0;
    record_file_scanf(filename, "number_of_arenas", "%*s %d", &num);
    if (num < 0 || num > ARENACONF_MAX_NUMBER_OF_ARENAS)
    {
        LOG("Invalid number of arenas %d\n", num);
        num = ARENACONF_MAX_NUMBER_OF_ARENAS;
    }
    config->number_of_arenas = num;

    for (int i = 0; i < num; i++)
    {
        char arena_key[100];
        sprintf(arena_key, "arena_%d", i);
        record_file_scanf(filename, arena_key, "%*s %s level_number=%d", config->arenas[i].name, &config->arenas[i].level_number);

        for (char *p = config->arenas[i].name; *p; p++)
        {
            if (*p == '_')
                *p = ' ';
        }

        LOG("Read arena config: '%s' = %d\n", config->arenas[i].name, config->arenas[i].level_number);
    }
}

void read_arena_highscores(const char *filename, ArenaHighscore *highscore)
{
    for (int i = 0; i < ARENACONF_MAX_NUMBER_OF_ARENAS; i++)
    {
        for (int mi = 0; mi < ARENACONF_HIGHSCORE_MAP_SIZE; mi++)
        {
            highscore->mode[i][mi] = 0;
            highscore->kills[i][mi] = 0;
            highscore->dirty[i][mi] = 0;
            char key[100];
            sprintf(key, "arena_%d_item_%d", i, mi);
            record_file_scanf(filename, key, "%*s mode=%d kills=%d", &highscore->mode[i][mi], &highscore->kills[i][mi]);
        }
    }
}

void write_arena_highscores(const char *filename, ArenaHighscore *highscore)
{
    for (int i = 0; i < ARENACONF_MAX_NUMBER_OF_ARENAS; i++)
    {
        for (int mi = 0; mi < ARENACONF_HIGHSCORE_MAP_SIZE; mi++)
        {
            if (highscore->dirty[i][mi])
            {
                record_file_set_record_f(filename, "arena_%d_item_%d mode=%d kills=%d",
                                         i, mi, highscore->mode[i][mi], highscore->kills[i][mi]);
            }
        }
    }
}