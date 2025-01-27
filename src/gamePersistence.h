#pragma once
#include "world.h"

/*
 * Same as save_game_save_data but with constant datadir\save.dat filename.
 */
void save_game(Enemy *autosave, int mission, int game_modifiers, int slot);

/*
 * Writes a game state to the provided file. Uses the record file format.
 * The syntax is:
 * slot_[slot]--[property] [value]
 * The following properties are saved:
 * game_modifiers, mission, health, shots, reload, rate, ammo, gold
 * (shots, reload, rate = weapon selection)
 */
void save_game_save_data(const char *filename, Enemy *data, int mission, int game_modifiers, int slot);

/*
 * Same as load_game_save_data but with constant datadir\save.dat filename.
 */
void load_game(Enemy *data, int *mission, int *game_modifiers, int slot);

/*
 * Reads a game state from the provided file. See save_game_save_data for more information.
 */
void load_game_save_data(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot);

/*
 * Reads the records slot_[slot]--game_modifiers and slot_[slot]--mission from the save file and sets
 * has_save to 1.
 * If the records don't exist, just sets has_save to 0.
 */
void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers);
