#include "arenaconf.h"

#include "duConstants.h"
#include "world.h"
#include "settings.h"
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

static void get_arena_highscores_path(char *path)
{
    sprintf(path, DATADIR "%s\\arcade_mode_highscores.dat", get_game_settings()->mission_pack);
}

static void get_arena_highscore_key(char *result, int mission, int game_mode)
{
    sprintf(result, "mission=%d;mode=%d;", mission, game_mode & ~GAMEMODIFIER_ARENA_FIGHT);
}

int get_arena_highscore(int mission, int game_mode)
{
    char path[256];
    get_arena_highscores_path(path);
    char key[64];
    get_arena_highscore_key(key, mission, game_mode);
    int score = 0;
    record_file_scanf(path, key, "%*s %d", &score);
    return score;
}

void set_arena_highscore(int mission, int game_mode, int score)
{
    char path[256];
    get_arena_highscores_path(path);
    char key[64];
    get_arena_highscore_key(key, mission, game_mode);
    record_file_set_record_f(path, "%s %d", key, score);
}