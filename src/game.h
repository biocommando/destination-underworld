#pragma once
#include "world.h"

/*
 * Play one game mission. Returns next mission to play.
 * -1 = go back to main menu.
 */
int game(int mission, int *game_modifiers, Enemy *plrautosave);

/*
 * Enables no player damage mode for debugging purposes.
 */
void enable_no_player_damage();