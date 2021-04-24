#ifndef SETTINGS_H
#define SETTINGS_H

typedef struct {
  char name[64], filename[256];
} NameToFilenameMapping;

typedef struct
{
  int missionCount;
  NameToFilenameMapping *missions;
  int screenWidth;
  int screenHeight;
  int screenMode;
  int vibrationMode;
} GameSettings;
void readSettings();
int readCmdLineArgInt(const char *arg, char **argv, int argc);
int readCmdLineArgStr(const char *arg, char **argv, int argc, char *output);

#endif
