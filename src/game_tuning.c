#include "game_tuning.h"
#include "logging.h"
#include "du_constants.h"
#include "logging.h"
#include "settings.h"
#include "read_level.h"

#include "command_file/generated/dispatch_game_tuning.h"

#include <stdio.h>
#include <string.h>

static GameTuningParams _tuning_params;

const GameTuningParams *get_tuning_params()
{
    return &_tuning_params;
}

void set_tuning_params(const GameTuningParams *gt)
{
    memcpy(&_tuning_params, gt, sizeof(GameTuningParams));
}

void read_game_tuning_params()
{
    char fname[256];
    sprintf(fname, DATADIR "%s/game-tuning.dat", get_game_settings()->mission_pack);
    FATAL(check_authentication(fname), "game tuning authentication failed!!\n");
    memset(&_tuning_params, 0, sizeof(_tuning_params));
    int err = read_command_file(fname, dispatch__game_tuning, &_tuning_params);
    FATAL(err != 0, "Failed to read required file: %s\n", fname);
    IF_command_file_RequiredFlags_NOT_SET(&_tuning_params.required_flags,
                                          {
                                              FATAL(1, "All tuning parameters need to be defined!\n");
                                          });
}