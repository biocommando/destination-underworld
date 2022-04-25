#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "settings.h"
#include "iniRead.h"

extern GameSettings game_settings;

void read_settings()
{
  FILE *f = fopen(".\\dataloss\\settings.ini", "r");
  ini_read_string_value(f, "general", "mission-pack", game_settings.mission_pack);
  game_settings.mission_count = ini_read_int_value(f, game_settings.mission_pack, "mission-count");
  game_settings.custom_resources = ini_read_int_value(f, game_settings.mission_pack, "custom-resources");

  game_settings.screen_width = ini_read_int_value(f, "graphics", "width");
  game_settings.screen_height = ini_read_int_value(f, "graphics", "height");
  game_settings.screen_mode = ini_read_int_value(f, "graphics", "screen");
  game_settings.vibration_mode = ini_read_int_value(f, "graphics", "vibration-mode");
  game_settings.music_on = ini_read_int_value(f, "audio", "music-on");
  game_settings.num_music_tracks = ini_read_int_value(f, "audio", "music-track-count");

  fclose(f);

  char arena_config_file[256];
  sprintf(arena_config_file, ".\\dataloss\\%s\\arenas.ini", game_settings.mission_pack);
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
  sprintf(path, ".\\dataloss\\%s\\arena_highscores.dat", game_settings.mission_pack);
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
