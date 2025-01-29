#include <stdio.h>
#include "gamePersistence.h"
#include "record_file.h"
#include "settings.h"

void load_game_save_data(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot)
{
    char rec[100], key[100];

    sprintf(key, "slot_%d--game_modifiers", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", game_modifiers);

    sprintf(key, "slot_%d--mission", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", mission);

    sprintf(key, "slot_%d--health", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", &data->health);

    sprintf(key, "slot_%d--shots", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", &data->shots);

    sprintf(key, "slot_%d--reload", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", &data->reload);

    sprintf(key, "slot_%d--rate", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", &data->rate);

    sprintf(key, "slot_%d--ammo", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", &data->ammo);

    sprintf(key, "slot_%d--gold", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", &data->gold);

    data->hurts_monsters = 1;
    data->sprite = -1;
    data->id = PLAYER_ID;
    data->former_id = PLAYER_ID;
}

void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers)
{
    char filename[100];
    sprintf(filename, SAVE_FILENAME, get_game_settings()->mission_pack);

    char rec[100], key[100];

    sprintf(key, "slot_%d--game_modifiers", slot);
    int ret = record_file_get_record(filename, key, rec, sizeof(rec));

    if (ret)
    {
        *has_save = 0;
        return;
    }

    sscanf(rec, "%*s %d", game_modifiers);

    sprintf(key, "slot_%d--mission", slot);
    record_file_get_record(filename, key, rec, sizeof(rec));
    sscanf(rec, "%*s %d", mission);

    *has_save = 1;
}

void save_game_save_data(const char *filename, Enemy *data, int mission, int game_modifiers, int slot)
{
    char rec[100], key[100];

    sprintf(key, "slot_%d--game_modifiers", slot);
    sprintf(rec, "%s %d", key, game_modifiers);
    record_file_set_record(filename, key, rec);

    sprintf(key, "slot_%d--mission", slot);
    sprintf(rec, "%s %d", key, mission);
    record_file_set_record(filename, key, rec);

    sprintf(key, "slot_%d--health", slot);
    sprintf(rec, "%s %d", key, data->health);
    record_file_set_record(filename, key, rec);

    sprintf(key, "slot_%d--shots", slot);
    sprintf(rec, "%s %d", key, data->shots);
    record_file_set_record(filename, key, rec);

    sprintf(key, "slot_%d--reload", slot);
    sprintf(rec, "%s %d", key, data->reload);
    record_file_set_record(filename, key, rec);

    sprintf(key, "slot_%d--rate", slot);
    sprintf(rec, "%s %d", key, data->rate);
    record_file_set_record(filename, key, rec);

    sprintf(key, "slot_%d--ammo", slot);
    sprintf(rec, "%s %d", key, data->ammo);
    record_file_set_record(filename, key, rec);

    sprintf(key, "slot_%d--gold", slot);
    sprintf(rec, "%s %d", key, data->gold);
    record_file_set_record(filename, key, rec);
}

void save_game(Enemy *autosave, int mission, int game_modifiers, int slot)
{
    char filename[100];
    sprintf(filename, SAVE_FILENAME, get_game_settings()->mission_pack);

    save_game_save_data(filename, autosave, mission, game_modifiers, slot);
}

void load_game(Enemy *data, int *mission, int *game_modifiers, int slot)
{
    char filename[100];
    sprintf(filename, SAVE_FILENAME, get_game_settings()->mission_pack);
    load_game_save_data(filename, data, mission, game_modifiers, slot);
}