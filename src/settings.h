#pragma once

#include "arenaconf.h"

typedef struct
{
  char settings_file[256];
  char mission_pack[64];
  int mission_count;
  int vibration_mode;
  int fullscreen;
  int music_on;
  float music_vol;
  float sfx_vol;
  int num_music_tracks;
  int custom_resources;
  ArenaConfigs arena_config;
} GameSettings;
void read_settings(char **argv, int argc);
void save_settings();
int read_cmd_line_arg_int(const char *arg, char **argv, int argc);
int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output);

void access_arena_highscore(ArenaHighscore *arena_highscore, int load);
// Destination is expected to be at least 256 characters
void get_data_filename(char *dst, const char *file);
