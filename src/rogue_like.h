#pragma once

#include "game_tuning.h"

typedef struct 
{
    double amount;
    int param_id;
} GameTuningModifier;

void modify_tuning_params(GameTuningParams *, const GameTuningModifier *);
double get_tuning_param_current_value(const GameTuningParams *params, int param_id);
const char *get_tuning_param_description(int param_id);

GameTuningModifier get_tuning_param_modifier(int index, int is_bad);
