#pragma once
#include "../../world.h"
#include "../../variables.h"
#include "required_flags.h"


typedef struct
{
    World *world;
    BossFightConfig *bfconfig;
    BossFightEventConfig *bfevent;
    VarState *variables;
    int has_boss;
    command_file_RequiredFlags required_flags;
} LevelState;
