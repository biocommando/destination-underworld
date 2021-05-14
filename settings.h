#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct
{
  char mission_pack[64];
  int mission_count;
  int screen_width;
  int screen_height;
  int screen_mode;
  int vibration_mode;
  int music_on;
  int num_music_tracks;
  int custom_resources;
} GameSettings;
void read_settings();
int read_cmd_line_arg_int(const char *arg, char **argv, int argc);
int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output);

#endif
