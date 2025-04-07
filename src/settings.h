#pragma once

#include "arenaconf.h"

// Overall game settings
typedef struct
{
  // This can be overridden from command line, defaults to datadir\settings.dat
  char settings_file[256];
  // Mission pack name (directory name added to data files)
  char mission_pack[64];
  // How violently the screen shakes on explosions. Smaller number means heavier shaking.
  int vibration_mode;
  // Window mode, 0 = windowed, 1 = fullscreen. Game must be restarted for this to take effect.
  int fullscreen;
  // Music on/off
  int music_on;
  // Music volume 0-1
  float music_vol;
  // Sound effect volume 0-1
  float sfx_vol;
  // Path to font file used for in-game info
  char game_font[256];
  // Path to font file used for menus
  char menu_font[256];
  // If custom resources are in use, this should be set to 1.
  // By default, different mission packs use the core game's sprites and sound effects but
  // if this is set to 1, also they are read from the `datadir\missionpack\` directory.
  int custom_resources;
  // If set to true, checks the dmac hashes for mission, configuration, recording and save files.
  // Refuses to open data and kills the game (not necessarily in a nice way) if hashes differ
  int require_authentication;
  // Arena fight configurations (cannot be accessed via command line arguments)
  ArenaConfigs arena_config;
} GameSettings;

/*
 * Read game settings. The settings are read from file settings.dat.
 * The file uses record file format with the following structure:
 * [setting id] [value]
 * The settings can be overridden with command line arguments using the following
 * syntax:
 * --[setting id]=[value]
 * For example, if setting id is audio--music-on, overriding this to 0 could be done
 * using command line argument `--audio--music-on=0`.
 *
 * The read result is saved to a global variable that can be using get_game_settings.
 */
void read_settings(char **argv, int argc);

/*
 * Saves settings similarly as in read_settings. Note that any command line overrides
 * will be saved as part of the settings file.
 */
void save_settings();
/*
 * Reads an argument using read_cmd_line_arg_str. Parses the output
 * as integer and returns the value. Returns 0 as the default value.
 */
int read_cmd_line_arg_int(const char *arg, char **argv, int argc, const char *documentation);
/*
 * Read command line argument from argv and argc passed from main function.
 * If there's an argument in format:
 * --[arg]=[value]
 * the value is set to output variable.
 * On success returns 1, otherwise 0.
 */
int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output, const char *documentation);
/*
 * Read or write ArenaHighscore structure from arena_highscores.dat.
 * The file uses record file format and the structure is:
 * arena_[arena_index]_item_[item_index] mode=%d kills=%d
 *
 * The format is not very friendly to access but it's only read and wrote automatically
 * by this function.
 *
 * Note that the key values are just for human-readability, the key-value order needs to be exactly like this.
 */
void access_arena_highscore(ArenaHighscore *arena_highscore, int load);
/*
 * Gets filename for a data file that may be a custom resource.
 * If custom resources are in use, sets the path to datadir\mission_pack\file,
 * otherwise sets to datadir\file.
 *
 * The destination buffer size should be at least strlen(DATADIR) + 64 + strlen(file) + 1.
 */
void get_data_filename(char *dst, const char *file);

/*
 * Get the GameSettings singleton instance. 
 */
GameSettings *get_game_settings();