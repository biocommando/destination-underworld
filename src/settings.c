#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logging.h"
#include "settings.h"
#include "record_file.h"
#include "duConstants.h"

static GameSettings game_settings;

static void read_setting(const char *filename, char **argv, int argc, char *result, const char *segment, const char *key)
{
  char cmd_line_arg[256];
  sprintf(cmd_line_arg, "%s--%s", segment, key);

  result[0] = 0;

  if (!read_cmd_line_arg_str(cmd_line_arg, argv, argc, result, "Setting read from settings.dat"))
  {
    record_file_scanf(filename, cmd_line_arg, "%*s %s", result);
  }
  LOG("Setting %s = %s\n", cmd_line_arg, result);
}

#define READ_SETTING(result_var, format_str, segment, key)                    \
  do                                                                          \
  {                                                                           \
    read_setting(game_settings.settings_file, argv, argc, buf, segment, key); \
    if (!buf[0] || sscanf(buf, format_str, result_var) == 0)                  \
      LOG("Read error\n");                                                    \
                                                                              \
  } while (0)

void read_settings(char **argv, int argc)
{
  char buf[256], file_name[256] = DATADIR "settings.dat";

  read_cmd_line_arg_str("settings-dat", argv, argc, file_name, "Filename (relative to work dir) to override settings.dat");
  strcpy(game_settings.settings_file, file_name);

  LOG("Settings file: %s\n", file_name);

  READ_SETTING(game_settings.mission_pack, "%s", "general", "mission-pack");
  READ_SETTING(&game_settings.custom_resources, "%d", game_settings.mission_pack, "custom-resources");
  READ_SETTING(&game_settings.require_authentication, "%d", game_settings.mission_pack, "require-authentication");

  READ_SETTING(&game_settings.vibration_mode, "%d", "graphics", "vibration-mode");
  READ_SETTING(&game_settings.fullscreen, "%d", "graphics", "fullscreen");
  READ_SETTING(game_settings.menu_font, "%s", "graphics", "menu-font");
  READ_SETTING(game_settings.game_font, "%s", "graphics", "game-font");

  READ_SETTING(&game_settings.music_on, "%d", "audio", "music-on");
  READ_SETTING(&game_settings.music_vol, "%f", "audio", "music-vol");
  READ_SETTING(&game_settings.sfx_vol, "%f", "audio", "sfx-vol");

  char arena_config_file[256];
  sprintf(arena_config_file, DATADIR "%s/arenas.dat", game_settings.mission_pack);
  read_arena_configs(arena_config_file, &game_settings.arena_config);
}

int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output, const char *documentation)
{
  char format_str[256];
  sprintf(format_str, "--%s=%%s", arg);

  printf(format_str, "<value>\n");
  if (documentation)
    printf("  %s\n", documentation);

  while (argc--)
  {
    int success = sscanf(argv[argc], format_str, output);
    if (success)
      return 1;
  }
  return 0;
}

int read_cmd_line_arg_int(const char *arg, char **argv, int argc, const char *documentation)
{
  char str[256];
  int success = read_cmd_line_arg_str(arg, argv, argc, str, documentation);
  if (success)
  {
    int value;
    success = sscanf(str, "%d", &value);
    if (success)
      return value;
  }
  return 0;
}

void get_data_filename(char *dst, const char *file)
{
  if (game_settings.custom_resources)
  {
    sprintf(dst, DATADIR "%s/%s", game_settings.mission_pack, file);
  }
  else
  {
    sprintf(dst, DATADIR "%s", file);
  }
}

void save_settings()
{
  record_file_set_record_f(game_settings.settings_file, "graphics--vibration-mode %d", game_settings.vibration_mode);
  record_file_set_record_f(game_settings.settings_file, "graphics--fullscreen %d", game_settings.fullscreen);
  record_file_set_record_f(game_settings.settings_file, "audio--music-on %d", game_settings.music_on);
  record_file_set_record_f(game_settings.settings_file, "audio--music-vol %lf", game_settings.music_vol);
  record_file_set_record_f(game_settings.settings_file, "audio--sfx-vol %lf", game_settings.sfx_vol);
}

inline GameSettings *get_game_settings()
{
  return &game_settings;
}