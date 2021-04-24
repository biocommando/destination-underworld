#include "gamePersistence.h"
#include "iniRead.h"

/*void NewUniqueGameId(struct gamedata *data)
{
    sprintf(data->gameId, "%d;%d", time(NULL), rand);
}

void NewUniqueGame(struct gamedata *data)
{
    NewUniqueGameId(data);
    data->kills = 0;
    data->deaths = 0;
    data->fireballs = 0;
    data->powerups = 0;
}*/

FILE *openFileFromSlot(int slot, const char *mode)
{
    char filename[50];
    sprintf(filename, SAVE_FILENAME, slot);
    return fopen(filename, mode);
}

void loadGameSaveData(FILE *f, Enemy *data, int *mission, int *gameModifiers)
{
    *gameModifiers = iniReadIntValue(f, "save_game", "game_modifiers");
    *mission = iniReadIntValue(f, "save_game", "mission");
    data->health = iniReadIntValue(f, "save_game", "health");
    data->shots = iniReadIntValue(f, "save_game", "shots");
    data->reload = iniReadIntValue(f, "save_game", "reload");
    data->rate = iniReadIntValue(f, "save_game", "rate");
    data->ammo = iniReadIntValue(f, "save_game", "ammo");
    data->gold = iniReadIntValue(f, "save_game", "gold");
    data->type = PLAYER;
}

void peekIntoSaveData(int slot, int *has_save, int *mission, int *game_modifiers)
{
    FILE *f = openFileFromSlot(slot, "r");
    if (!f)
    {
       *has_save = 0;
       return;
    }
    *has_save = 1;
    *mission = iniReadIntValue(f, "save_game", "mission");
    *game_modifiers = iniReadIntValue(f, "save_game", "game_modifiers");
    fclose(f);
}

/*void loadUniqueGameData(FILE *f, struct gamedata *data)
{
    iniReadCharValue(f, "unique_data", "game_id", data->gameId);
    data->kills = iniReadIntValue(f, "unique_data", "kills");
    data->deaths = iniReadIntValue(f, "unique_data", "deaths");
    data->fireballs = iniReadIntValue(f, "unique_data", "fireballs");
    data->powerups = iniReadIntValue(f, "unique_data", "powerups");
}*/

void saveGameSaveData(FILE *f, Enemy *data, int mission, int gameModifiers)
{
    fprintf(f, "[save_game]\nmission=%d\nhealth=%d\nshots=%d\nreload=%d\nrate=%d\nammo=%d\ngold=%d\ngame_modifiers=%d\n",
            mission, data->health, data->shots, data->reload, data->rate, data->ammo, data->gold, gameModifiers);
}

/*void saveUniqueGameData(FILE *f, struct gamedata *data)
{
    fprintf(f, "[unique_data]\ngame_id=%s\n", data->gameId);
    fprintf(f, "kills=%d\ndeaths=%d\nfireballs=%d\npowerups=%d\n",
            data->kills, data->deaths, data->fireballs, data->powerups);
}*/

void saveGame(Enemy *autosave, int mission, int gameModifiers, int slot)
{
    /*char filename[50];
    sprintf(filename, SAVE_FILENAME, slot);
    FILE *f = fopen(filename, "w");*/
    FILE *f = openFileFromSlot(slot, "w");
//    saveUniqueGameData(f, uniqueData);
    saveGameSaveData(f, autosave, mission, gameModifiers);
    fclose(f);
}

/*void StoreUniqueGame(struct gamedata *uniqueData)
{
    FILE *f = fopen(SAVE_FILENAME, "r");
    struct gamedata savedUniqueGame;
    loadUniqueGameData(f, &savedUniqueGame);
    if (strcmp(uniqueData->gameId, savedUniqueGame.gameId))
    {
        fclose(f);
        return;
    }
    Enemy saveData;
    int mission;
    int gameModifiers;
    loadGameSaveData(f, &saveData, &mission, &gameModifiers);
    fclose(f);

    saveGame(&saveData, uniqueData, mission, gameModifiers);
}*/
