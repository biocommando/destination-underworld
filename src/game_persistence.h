#pragma once
#include "world.h"


/*
 * Writes a game state to the provided file. Uses the record file format.
 * The syntax is:
 * key:
 * - slot_[slot]
 * parameters:
 * - game_modifiers
 * - mission
 * - health
 * - shots
 * - reload
 * - rate
 * - ammo
 * - gold
 * - perks
 * - xp
 * - salt (only if authentication required)
 * - hash (only if authentication required)
 *
 * (shots, reload, rate = weapon selection)
 * 
 * If filename is NULL, uses default datadir\save.dat filename.
 */
void save_game(const char *filename, Enemy *autosave, int mission, int game_modifiers, int slot);

/*
 * Reads a game state from the provided file. See save_game_save_data for more information.
 * 
 * If filename is NULL, uses default datadir\save.dat filename.
 */
void load_game(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot);

/*
 * Reads the records slot_[slot]--game_modifiers and slot_[slot]--mission from the save file and sets
 * has_save to 1.
 * If the records don't exist, just sets has_save to 0.
 */
void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers, int *is_rogue_like);

int load_rogue_like_modifiers(const char *filename, int slot, LinkedList *dst, int *gimmicks);
void save_rogue_like_modifiers(const char *filename, int slot, LinkedList *src, int gimmicks);