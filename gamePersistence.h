#ifndef GAMESAVE_H
#define GAMESAVE_H
#include <stdio.h>
#include "world.h"

//void StoreUniqueGame(struct gamedata *uniqueData);
void saveGame(Enemy *autosave, int mission, int gameModifiers, int slot);
//void saveUniqueGameData(FILE *f, struct gamedata *data);
void saveGameSaveData(FILE *f, Enemy *data, int mission, int gameModifiers);
//void loadUniqueGameData(FILE *f, struct gamedata *data);
void loadGameSaveData(FILE *f, Enemy *data, int *mission, int *gameModifiers);
//void NewUniqueGame(struct gamedata *uniqueData);
//void NewUniqueGameId(struct gamedata *uniqueData);

void peekIntoSaveData(int slot, int *has_save, int *mission, int *game_modifiers);

#endif
