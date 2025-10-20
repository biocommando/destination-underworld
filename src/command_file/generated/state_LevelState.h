#pragma once
#include "../../world.h"
#include "../../variables.h"

typedef struct
{
    World *world;
    BossFightConfig *bfconfig;
    BossFightEventConfig *bfevent;
    VarState *variables;
} LevelState;
