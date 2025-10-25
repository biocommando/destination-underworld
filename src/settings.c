#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logging.h"
#include "settings.h"
#include "record_file.h"
#include "duConstants.h"
#include "allegro_management.h"

static GameSettings game_settings;

static void read_setting(const char *filename, char **argv, int argc, char *result, const char *segment, const char *key)
{
  char cmd_line_arg[256];
  sprintf(cmd_line_arg, "%s--%s", segment, key);

  result[0] = 0;

  if (!read_cmd_line_arg_str(cmd_line_arg, argv, argc, result, "Setting read from settings.dat"))
  {
    record_file_find_and_read(filename, cmd_line_arg);
    const char *value = record_file_next_param();
    if (value)
      strncpy(result, value, 255);
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

static void read_keys()
{
  // Set defaults
  game_settings.keys.left = ALLEGRO_KEY_LEFT;
  game_settings.keys.right = ALLEGRO_KEY_RIGHT;
  game_settings.keys.up = ALLEGRO_KEY_UP;
  game_settings.keys.down = ALLEGRO_KEY_DOWN;
  game_settings.keys.shoot = ALLEGRO_KEY_SPACE;

  game_settings.keys.weapon0 = ALLEGRO_KEY_Z;
  game_settings.keys.weapon1 = ALLEGRO_KEY_X;

  game_settings.keys.pwup0 = ALLEGRO_KEY_A;
  game_settings.keys.pwup1 = ALLEGRO_KEY_S;
  game_settings.keys.pwup2 = ALLEGRO_KEY_D;
  game_settings.keys.pwup3 = ALLEGRO_KEY_F;

  game_settings.keys.restart = ALLEGRO_KEY_R;
  game_settings.keys.map_info = ALLEGRO_KEY_M;

  record_file_find_and_read(game_settings.settings_file, "key-bindings");
  game_settings.keys.left = record_file_next_param_as_int(game_settings.keys.left);
  game_settings.keys.right = record_file_next_param_as_int(game_settings.keys.right);
  game_settings.keys.up = record_file_next_param_as_int(game_settings.keys.up);
  game_settings.keys.down = record_file_next_param_as_int(game_settings.keys.down);
  game_settings.keys.shoot = record_file_next_param_as_int(game_settings.keys.shoot);
  game_settings.keys.weapon0 = record_file_next_param_as_int(game_settings.keys.weapon0);
  game_settings.keys.weapon1 = record_file_next_param_as_int(game_settings.keys.weapon1);
  game_settings.keys.pwup0 = record_file_next_param_as_int(game_settings.keys.pwup0);
  game_settings.keys.pwup1 = record_file_next_param_as_int(game_settings.keys.pwup1);
  game_settings.keys.pwup2 = record_file_next_param_as_int(game_settings.keys.pwup2);
  game_settings.keys.pwup3 = record_file_next_param_as_int(game_settings.keys.pwup3);
  game_settings.keys.restart = record_file_next_param_as_int(game_settings.keys.restart);
  game_settings.keys.map_info = record_file_next_param_as_int(game_settings.keys.map_info);
}

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

  read_keys();
}

int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output, const char *documentation)
{
  char format_str[256] = "";
  if (strlen(arg) < sizeof(format_str) - 5)
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
  record_file_find_and_modify(game_settings.settings_file, "graphics--vibration-mode");
  record_file_add_int_param(game_settings.vibration_mode);
  record_file_find_and_modify(game_settings.settings_file, "graphics--fullscreen");
  record_file_add_int_param(game_settings.fullscreen);
  record_file_find_and_modify(game_settings.settings_file, "audio--music-on");
  record_file_add_int_param(game_settings.music_on);
  record_file_find_and_modify(game_settings.settings_file, "audio--music-vol");
  record_file_add_float_param(game_settings.music_vol);
  record_file_find_and_modify(game_settings.settings_file, "audio--sfx-vol");
  record_file_add_float_param(game_settings.sfx_vol);

  record_file_find_and_modify(game_settings.settings_file, "key-bindings");
  record_file_add_int_param(game_settings.keys.left);
  record_file_add_int_param( game_settings.keys.right);
  record_file_add_int_param( game_settings.keys.up);
  record_file_add_int_param( game_settings.keys.down);
  record_file_add_int_param( game_settings.keys.shoot);
  record_file_add_int_param( game_settings.keys.weapon0);
  record_file_add_int_param( game_settings.keys.weapon1);
  record_file_add_int_param( game_settings.keys.pwup0);
  record_file_add_int_param( game_settings.keys.pwup1);
  record_file_add_int_param( game_settings.keys.pwup2);
  record_file_add_int_param( game_settings.keys.pwup3);
  record_file_add_int_param( game_settings.keys.restart);
  record_file_add_int_param( game_settings.keys.map_info);
}

inline GameSettings *get_game_settings()
{
  return &game_settings;
}
