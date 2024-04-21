#pragma once
#include <stdio.h>
#include "world.h"

void save_game(Enemy *autosave, int mission, int game_modifiers, int slot);
void save_game_save_data(const char *filename, Enemy *data, int mission, int game_modifiers, int slot);
void load_game(Enemy *data, int *mission, int *game_modifiers, int slot);
void load_game_save_data(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot);

void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers);
