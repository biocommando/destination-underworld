#include "arenaconf.h"

#include "duConstants.h"
#include "world.h"
#include "settings.h"
#include "record_file.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>
#include "command_file/generated/dispatch_arena_conf.h"

void dispatch__handle_arena_conf_add(struct arena_conf_add_DispatchDto *dto)
{
    int idx = dto->state->number_of_arenas;
    if (idx == ARENACONF_MAX_NUMBER_OF_ARENAS)
    {
        LOG("Max number of arenas %d exceeded!!\n", ARENACONF_MAX_NUMBER_OF_ARENAS);
        return;
    }
    dto->state->arenas[idx].level_number = dto->level_number;
    strcpy(dto->state->arenas[idx].name, dto->name);
    LOG("Read arena config: '%s' = %d\n", dto->state->arenas[idx].name, dto->state->arenas[idx].level_number);
    dto->state->number_of_arenas++;
}

void read_arena_configs(const char *filename, ArenaConfigs *config)
{
    memset(config, 0, sizeof(ArenaConfigs));
    read_command_file(filename, dispatch__arena_conf, config);
    /*unsigned num = 0;
    record_file_scanf(filename, "number_of_arenas", "%*s %u", &num);
    if (num > ARENACONF_MAX_NUMBER_OF_ARENAS)
    {
        LOG("Invalid number of arenas %d\n", num);
        num = ARENACONF_MAX_NUMBER_OF_ARENAS;
    }
    config->number_of_arenas = num;

    for (unsigned i = 0; i < num; i++)
    {
        char arena_key[100];
        sprintf(arena_key, "arena_%u", i);
        char record[256];
        if (record_file_get_record(filename, arena_key, record, sizeof(record)) != 0)
            continue;
        sscanf(record, "%*s level_number=%d", &config->arenas[i].level_number);
        char *name = strstr(record, "name=");
        if (name && strlen(name + 5) < sizeof(config->arenas[i].name))
        {
            name += 5;
            strcpy(config->arenas[i].name, name);
        }
        else
        {
            sprintf(config->arenas[i].name, "Arena level %u", i + 1);
        }
        LOG("Read arena config: '%s' = %d\n", config->arenas[i].name, config->arenas[i].level_number);
    }*/
}

static void get_arena_highscores_path(char *path)
{
    sprintf(path, DATADIR "%s/arcade_mode_highscores.dat", get_game_settings()->mission_pack);
}

static void get_arena_highscore_key(char *result, int mission, int game_mode)
{
    sprintf(result, "level_number=%d;mode=%d;", mission, game_mode & ~GAMEMODIFIER_ARENA_FIGHT);
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