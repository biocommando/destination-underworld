#pragma once

#include "world.h"

/*
 * Shows ingame or main menu (controlled by parameter). 
 * The mission number that will be loaded if the function returns 1 is saved to ggs;
 * if the function returns 0 the level won't be switched. The function modifies
 * GlobalGameState.
 */
int menu(int ingame, GlobalGameState *ggs);

struct custom_flat_menu_item
{
    char name[100];
    char description[300];
};

int custom_flat_menu(const char *title, const struct custom_flat_menu_item *menu, size_t num_items, int has_cancel);