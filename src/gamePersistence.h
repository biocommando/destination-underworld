#ifndef GAMESAVE_H
#define GAMESAVE_H
#include <stdio.h>
#include "world.h"

void save_game(Enemy *autosave, int mission, int game_modifiers, int slot);
void save_game_save_data(FILE *f, Enemy *data, int mission, int game_modifiers);
void load_game_save_data(FILE *f, Enemy *data, int *mission, int *game_modifiers);

void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers);

#endif
