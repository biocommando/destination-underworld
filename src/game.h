#pragma once
#include "world.h"

/*
 * Play one game mission.
 * The way the menus during and after the game are handled is a bit messy and the
 * game function may be called recursively.
 */
void game(GlobalGameState *ggs);