#pragma once
#include "../../world.h"
#include "../../variables.h"

typedef struct
{
    World *world;
    BossFightConfig *bfconfig;
    BossFightEventConfig *bfevent;
    VarState *variables;
    int has_boss;
} LevelState;
