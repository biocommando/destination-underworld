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
 * The level file uses command file format.
 *
 * The following commands are supported:
 * - header: "version" "extra" (required)
 *   * version contains the current version. This must be set to the expected value.
 *   * extra contains any metadata that can be used by e.g. level editor
 * - object: "id" "x" "y" "room"
 *   * The object id is a predefined 32 bit integer number that identifies the object.
 *     All gamedata is defined as objects (enemies, potions, walls etc.).
 * - condition: "expression" "label"
 *   * expression syntax: `<variable> <operator> <value> (and <variable> <operator> <value>...)`
 *   * if expression is true, set the skip label to parameter label
 * - goto: "label"
 *   * unconditionally set the skip label to parameter label
 * - set_var: "name" "value"
 *   * set variable to the string value (see supported variables below)
 * The lines that are not parsed by the duscript engine (i.e. game data) are handled by
 * this function. There are a couple of different special cases:
 * - The file should start with a line that starts with 'X'
 * - Lines that begin with '#' are skipped as these are level editor metadata
 * - Line in format `$NUMBER`. This means that the next lines contain a script that should
 *   be run in room id NUMBER (starting from 1). The data is read using read_bfconfig
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
 * - mute_bosstalk = don't play the boss speech sample if there's a boss in the level
 * - floor_color_base = the RGB color multiplier of the floors (each color either 0 or 1) in format `RED GREEN BLUE`.
 * - story_image = use custom image for the end screen
 */
int read_level(World *world, int mission, int room_to);
/*
 * Read enemy configurations from file enemy-properties.dat.
 * Uses the command file format with the following structure:
 * "type" "turret" "rate" "health" "gold" "fast" "hurts-monsters" "potion-for-potion-only"
 *
 * Note that the key values are just for human-readability, the key-value order needs to be exactly like this.
 */
void read_enemy_configs(World *world);
/*
 * Read mission count for the provided game mode.
 * Uses the command file format. Has the following commands:
 * - initial_count: "count" (required)
 *   * This is the base mission count
 * - mode_override: "mode" "count"
 *   * Override the base value for the game mode
 */
int read_mission_count(int game_mode);

/*
 * Reads GameTuningParams structure from a command file with syntax:
 * <field_name>: "value" (required)
 * Example:
 * max_health: "6"
 *
 * See all the field names in game_tuning.h.
 *
 * The file read is <mission pack datadir>/game-tuning.dat.
 */
void read_game_tuning_params();
