#include <stdio.h>
#include <stdlib.h>
#include "settings.h"
#include "iniRead.h"

extern GameSettings gameSettings;

void readSettings()
{
  FILE *f = fopen(".\\dataloss\\settings.ini", "r");
  gameSettings.missionCount = iniReadIntValue(f, "general", "missions");
  char missionPack[64];
  iniReadCharValue(f, "general", "mission-pack", missionPack);
  gameSettings.missions = (NameToFilenameMapping*) malloc(gameSettings.missionCount * sizeof(NameToFilenameMapping));
  for(int i = 0; i < gameSettings.missionCount; i++) 
  {
    char readValue[256];
    char keyToRead[64];
    sprintf(keyToRead, "mission%d", i + 1);
    iniReadCharValue(f, missionPack, keyToRead, readValue);
    sprintf(gameSettings.missions[i].filename, ".\\dataloss\\%s", readValue);
    sprintf(keyToRead, "mission%d-name", i + 1);
    iniReadCharValue(f, missionPack, keyToRead, gameSettings.missions[i].name);
  }
  gameSettings.screenWidth = iniReadIntValue(f, "graphics", "width");
  gameSettings.screenHeight = iniReadIntValue(f, "graphics", "height");
  gameSettings.screenMode = iniReadIntValue(f, "graphics", "screen");
  gameSettings.vibrationMode = iniReadIntValue(f, "graphics", "vibration-mode");
  
  fclose(f);
}

int readCmdLineArgStr(const char *arg, char **argv, int argc, char *output)
{
  char formatStr[256];
  sprintf(formatStr, "--%s=%%s", arg);
  while(argc--)
  {
    int success = sscanf(argv[argc], formatStr, output);
    if (success) return 1;
  }
  return 0;
}

int readCmdLineArgInt(const char *arg, char **argv, int argc)
{
  char str[256];
  int success = readCmdLineArgStr(arg, argv, argc, str);
  if (success)
  {
    int value;
    success = sscanf(str, "%d", &value);
    if (success) return value;
  }
  return 0;
}

