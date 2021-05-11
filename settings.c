#include <stdio.h>
#include <stdlib.h>
#include "settings.h"
#include "iniRead.h"

extern GameSettings game_settings;

void read_settings()
{
  FILE *f = fopen(".\\dataloss\\settings.ini", "r");
  ini_read_string_value(f, "general", "mission-pack", game_settings.mission_pack);
  game_settings.mission_count = ini_read_int_value(f, game_settings.mission_pack, "mission_count");
  game_settings.missions = (NameToFilenameMapping*) malloc(game_settings.mission_count * sizeof(NameToFilenameMapping));
  for(int i = 0; i < game_settings.mission_count; i++) 
  {
    char key_to_read[64];
    sprintf(game_settings.missions[i].filename, ".\\dataloss\\%s\\mission%d", game_settings.mission_pack, i + 1);
    sprintf(key_to_read, "mission%d-name", i + 1);
    ini_read_string_value(f, game_settings.mission_pack, key_to_read, game_settings.missions[i].name);
  }
  game_settings.screen_width = ini_read_int_value(f, "graphics", "width");
  game_settings.screen_height = ini_read_int_value(f, "graphics", "height");
  game_settings.screen_mode = ini_read_int_value(f, "graphics", "screen");
  game_settings.vibration_mode = ini_read_int_value(f, "graphics", "vibration-mode");
  game_settings.music_on = ini_read_int_value(f, "audio", "music-on");
  game_settings.num_music_tracks =  ini_read_int_value(f, "audio", "music-track-count");
  
  fclose(f);
}

int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output)
{
  char format_str[256];
  sprintf(format_str, "--%s=%%s", arg);
  while(argc--)
  {
    int success = sscanf(argv[argc], format_str, output);
    if (success) return 1;
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
    if (success) return value;
  }
  return 0;
}

