#ifndef KEYHANDLING_H
#define KEYHANDLING_H

#include "continuousData.h"
#include "world.h"


void getKeyPresses(ContinuousData *data, void *keyPressOutput);

int handleDirectionKeys(World *world, int keyUp, int keyDown, int keyLeft, int keyRight);
int handleWeaponChangeKeys(World *world, int keyX, int keyZ);
int handlePowerUpKeys(World *world, int keyA, int keyS, int keyD, int keyF, int *goldHintAmount, int *plr_rune_of_protection_active);
int handleShootKey(World *world, int keySpace);

#endif
