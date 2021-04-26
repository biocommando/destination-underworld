#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
  char name[64], filename[256];
} NameToFilenameMapping;

typedef struct
{
  int mission_count;
  NameToFilenameMapping *missions;
  int screen_width;
  int screen_height;
  int screen_mode;
  int vibration_mode;
} GameSettings;
void read_settings();
int read_cmd_line_arg_int(const char *arg, char **argv, int argc);
int read_cmd_line_arg_str(const char *arg, char **argv, int argc, char *output);

#endif
