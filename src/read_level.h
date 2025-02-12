#pragma once
#include "world.h"

/*
 * Read level from a file. The file name is either:
 * [GameSettings.mission_pack]\mission[mission]-mode-[World.game_modifiers]
 * or if that's not found:
 * [GameSettings.mission_pack]\mission[mission]
 *
 * The level is read once, and that will set the world state for all rooms.
 * The function has the room_to parameter which is a remnant from old code where
 * levels were read only when entering a new unvisited room.
 *
 * The level file uses duscript format which is a simple scripting format that
 * has a simple support for variables and condition based branching.
 *
 * The lines that are not parsed by the duscript engine (i.e. game data) are handled by
 * this function. There are a couple of different special cases:
 * - The file should start with a line that starts with 'X'
 * - Lines that begin with '#' are skipped as these are level editor metadata
 * - Line in format `$NUMBER`. This means that the next lines contain a script that should
 *   be run in room id NUMBER (starting from 1). The data is read using read_bfconfig_new
 *   that returns when the line `end` is read.
 * - Other lines are considered map data. Data is in format `object_id x y room_id`.
 *   The object id is a predefined 32 bit integer number that identifies the object.
 *   All gamedata is defined as objects (enemies, potions, walls etc.).
 *   One little oddity: the player is placed into room 1 at tile that is an entrance
 *   to room 1
 *
 * The following duscript variables are attempted to read after reading the level file:
 * - name = level display name (max 63 characters)
 * - storyX = lines of story text that is displayed after level (max 60 characters. X is a number
 *   from 0 to 10)
 * - wall_color = the RGB color (color values 0-1) of the walls, in format `RED GREEN BLUE` (e.g. `1.0 0.0 0.0` for red)
 * - no_more_levels = this is the final level; no more levels attempted to read
 * - mute_bosstalk = don't play the boss speech sample if there's a script in the level
 */
int read_level(World *world, int mission, int room_to);