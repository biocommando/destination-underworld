#pragma once

#include "world.h"

/*
 * Contains logic run each game frame for pickable potions.
 * Checks if the potion is picked up and applies its effects and draws the sprites and
 * the bubbling effect.
 */
void potion_logic(World *w);