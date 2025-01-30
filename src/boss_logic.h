#pragma once

#include "world.h"

/*
 * Contains the scripting logic that's run each 3rd game frame.
 * Checks the event triggers and handles the triggered events.
 */
void boss_logic(World *world, int boss_died);