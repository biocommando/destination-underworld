#pragma once

#include "world.h"

/*
 * Shows ingame or main menu (controlled by parameter). The autosave parameter should contain
 * the player state to which the player is initialized when starting the game.
 * The parameter mission is the mission number that will be loaded if the function returns 1;
 * if the function returns 0 the level won't be switched. The parameter game_modifiers
 * will contain the new game modifiers if a new game is started or a game state is loaded.
 */
int menu(int ingame, Enemy *autosave, int *mission, int *game_modifiers);
