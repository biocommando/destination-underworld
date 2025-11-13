#pragma once

#include "required_flags.h"

typedef struct {
    int game_mode;
    int count;
    int override_set;

    command_file_RequiredFlags required_flags;
} SetMissionCount;
