#include "gamePersistence.h"
#include "iniRead.h"
#include "settings.h"

extern GameSettings game_settings;

FILE *open_file_from_slot(int slot, const char *mode)
{
    char filename[50];
    sprintf(filename, SAVE_FILENAME, game_settings.mission_pack, slot);
    return fopen(filename, mode);
}

void load_game_save_data(FILE *f, Enemy *data, int *mission, int *game_modifiers)
{
    *game_modifiers = ini_read_int_value(f, "save_game", "game_modifiers");
    *mission = ini_read_int_value(f, "save_game", "mission");
    data->health = ini_read_int_value(f, "save_game", "health");
    data->shots = ini_read_int_value(f, "save_game", "shots");
    data->reload = ini_read_int_value(f, "save_game", "reload");
    data->rate = ini_read_int_value(f, "save_game", "rate");
    data->ammo = ini_read_int_value(f, "save_game", "ammo");
    data->gold = ini_read_int_value(f, "save_game", "gold");
    data->hurts_monsters = 1;
}

void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers)
{
    FILE *f = open_file_from_slot(slot, "r");
    if (!f)
    {
       *has_save = 0;
       return;
    }
    *has_save = 1;
    *mission = ini_read_int_value(f, "save_game", "mission");
    *game_modifiers = ini_read_int_value(f, "save_game", "game_modifiers");
    fclose(f);
}

void save_game_save_data(FILE *f, Enemy *data, int mission, int game_modifiers)
{
    fprintf(f, "[save_game]\nmission=%d\nhealth=%d\nshots=%d\nreload=%d\nrate=%d\nammo=%d\ngold=%d\ngame_modifiers=%d\n",
            mission, data->health, data->shots, data->reload, data->rate, data->ammo, data->gold, game_modifiers);
}

void save_game(Enemy *autosave, int mission, int game_modifiers, int slot)
{
    FILE *f = open_file_from_slot(slot, "w");
    save_game_save_data(f, autosave, mission, game_modifiers);
    fclose(f);
}
