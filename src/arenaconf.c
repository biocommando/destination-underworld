#include "arenaconf.h"

#include "du_constants.h"
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
    record_file_find_and_read(path, key);
    return record_file_next_param_as_int(0);
}

void set_arena_highscore(int mission, int game_mode, int score)
{
    char path[256];
    get_arena_highscores_path(path);
    char key[64];
    get_arena_highscore_key(key, mission, game_mode);
    record_file_find_and_modify(path, key);
    record_file_add_int_param(score);
}