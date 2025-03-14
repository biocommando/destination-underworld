#pragma once

#include "world.h"

/*
 * Checks the arrow key statuses and sets the player direction and moves the player accordingly.
 * Returns 1 if player direction changed.
 */
int handle_direction_keys(World *world, int key_up, int key_down, int key_left, int key_right);
/*
 * Checks the X and Z keys and changes the player weapon selection property changes.
 *
 * This function also handles sample playback.
 */
void handle_weapon_change_keys(World *world, int key_x, int key_z);
/*
 * Checks the A, S, D and F keys, and applies powerups and handles the gold economics accordingly.
 * The gold_hint_amount parameter is set to the powerup cost value if one of the powerups is selected.
 *
 * This function also handles sample playback.
 */
void handle_power_up_keys(World *world, int key_a, int key_s, int key_d, int key_f, int *gold_hint_amount);
/*
 * Checks the space key and makes the player shoot using the shoot(Enemy*, World*) function.
 *
 * This function also handles sample playback.
 */
void handle_shoot_key(World *world, int key_space);
