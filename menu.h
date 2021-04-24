#ifndef DU_MENU_H
#define DU_MENU_H
#include "world.h"

#define MENUOPT_NEW_GAME 0
#define MENUOPT_LOAD 1
#define MENUOPT_SAVE 2
#define MENUOPT_EXIT 3
#define MENUOPT_RESUME 4


int menu(int ingame, Enemy *autosave, int *mission, int *gameModifiers);

#endif
