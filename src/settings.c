#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "logging.h"
#include "settings.h"
#include "iniRead.h"
#include "duConstants.h"

extern GameSettings game_settings;

static void read_setting(FILE *f, char **argv, int argc, char *result, const char *segment, const char *key)
{
  char cmd_line_arg[256];
  sprintf(cmd_line_arg, "%s--%s", segment, key);

  result[0] = 0;

  if (!read_cmd_line_arg_str(cmd_line_arg, argv, argc, result) && f)
  {
    ini_read_string_value(f, segment, key, result);
  }
  LOG("Setting %s = %s\n", cmd_line_arg, result);
}

#define READ_SETTING(result_var, format_str, segment, key) do{ \
  read_setting(f, argv, argc, buf, segment, key); \
  if (!buf[0] || sscanf(buf, format_str, &result_var) == 0) \
     LOG("Read error\n"); \
   \
} while(0)

void read_settings(char **argv, int argc)
{
  char buf[256], file_name[256] = DATADIR "settings.ini";

  read_cmd_line_arg_str("settings-ini", argv, argc, file_name);

  LOG("Settings file: %s\n", file_name);
  FILE *f = fopen(file_name, "r");

  READ_SETTING(game_settings.mission_pack, "%s", "general", "mission-pack");
  READ_SETTING(game_settings.mission_count, "%d", game_settings.mission_pack, "mission-count");
  READ_SETTING(game_settings.custom_resources, "%d", game_settings.mission_pack, "custom-resources");

  READ_SETTING(game_settings.screen_width, "%d", "graphics", "width");
  READ_SETTING(game_settings.screen_height, "%d", "graphics", "height");
  READ_SETTING(game_settings.screen_mode, "%d", "graphics", "screen");
  READ_SETTING(game_settings.vibration_mode, "%d", "graphics", "vibration-mode");

  READ_SETTING(game_settings.music_on, "%d", "audio", "music-on");
  READ_SETTING(game_settings.music_vol, "%f", "audio", "music-vol");
  READ_SETTING(game_settings.sfx_vol, "%f", "audio", "sfx-vol");

  READ_SETTING(game_settings.num_music_tracks, "%d", "audio", "music-track-count");

  fclose(f);

  char arena_config_file[256];
  sprintf(arena_config_file, DATADIR "%s\\arenas.ini", game_settings.mission_pack);
  f = fopen(arena_config_file, "r");
  read_arena_configs(f, &game_settings.arena_config);
  fclose(f);
}

int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output)
{
  char format_str[256];
  sprintf(format_str, "--%s=%%s", arg);
  while (argc--)
  {
    int success = sscanf(argv[argc], format_str, output);
    if (success)
      return 1;
  }
  return 0;
}

int read_cmd_line_arg_int(const char *arg, char **argv, int argc)
{
  char str[256];
  int success = read_cmd_line_arg_str(arg, argv, argc, str);
  if (success)
  {
    int value;
    success = sscanf(str, "%d", &value);
    if (success)
      return value;
  }
  return 0;
}

void access_arena_highscore(ArenaHighscore *arena_highscore, int load)
{
  char path[256];
  sprintf(path, DATADIR "%s\\arena_highscores.dat", game_settings.mission_pack);
  FILE *f;
  if (!load)
  {
    f = fopen(path, "w");
    write_arena_highscores(f, arena_highscore);
    fclose(f);
    return;
  }
  f = fopen(path, "r");
  if (!f)
  {
    f = fopen(path, "w");
    for (int i = 0; i < ARENACONF_MAX_NUMBER_OF_ARENAS; i++)
    {
      for (int j = 0; j < ARENACONF_HIGHSCORE_MAP_SIZE; j++)
      {
        arena_highscore->kills[i][j] = 0;
        arena_highscore->mode[i][j] = -1;
      }
    }
    write_arena_highscores(f, arena_highscore);
    fclose(f);
    f = fopen(path, "r");
  }
  read_arena_highscores(f, arena_highscore);
  fclose(f);
}
