#pragma once

#include "world.h"

/*
 * Contains logic that is run every game frame for bullets (fireballs).
 * Contains logic for checking if bullet hits anything, damage calculation
 * and any damage consequences.
 */
void bullet_logic(World *world, GlobalGameState *ggs);