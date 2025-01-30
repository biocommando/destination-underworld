#pragma once

#include "world.h"

/*
 * Shows ingame or main menu (controlled by parameter). 
 * The mission number that will be loaded if the function returns 1 is saved to ggs;
 * if the function returns 0 the level won't be switched. The function modifies
 * GlobalGameState.
 */
int menu(int ingame, GlobalGameState *ggs);
