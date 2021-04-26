#ifndef KEYHANDLING_H
#define KEYHANDLING_H

#include "continuousData.h"
#include "world.h"


void get_key_presses(ContinuousData *data, void *key_press_output);

int handle_direction_keys(World *world, int key_up, int key_down, int key_left, int key_right);
int handle_weapon_change_keys(World *world, int key_x, int key_z);
int handle_power_up_keys(World *world, int key_a, int key_s, int key_d, int key_f, int *gold_hint_amount);
int handle_shoot_key(World *world, int key_space);

#endif
