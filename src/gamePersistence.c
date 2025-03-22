#include <stdio.h>
#include "gamePersistence.h"
#include "record_file.h"
#include "settings.h"

#include "sha1/du_dmac.h"

void create_verification_hash(char *hash_hex, int mission, int game_modifiers, const Enemy *data, int salt)
{
    char state_string[200];
    memset(state_string, 0, sizeof(state_string));
    sprintf(state_string, "game data %d %d %d %d %d %d %d %d %d %d salt=%d",
            game_modifiers, mission,
            data->health, data->shots, data->reload,
            data->rate, data->ammo, data->gold, data->perks,
            data->xp, salt);
    char hash[DMAC_SHA1_HASH_SIZE];
    dmac_sha1_calculate_hash(hash, state_string, sizeof(state_string));
    convert_sha1_hash_to_hex(hash_hex, hash);
}

static inline void read_save_data_record(const char *filename, int slot, const char *suffix, const char *format, void *dst)
{
    char key[100];
    sprintf(key, "slot_%d--%s", slot, suffix);
    char fullfmt[32];
    sprintf(fullfmt, "%%*s %s", format);
    record_file_scanf(filename, key, fullfmt, dst);
}

void load_game_save_data(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot)
{
    char rec[100], key[100];
    memset(data, 0, sizeof(Enemy));

    read_save_data_record(filename, slot, "game_modifiers", "%d", game_modifiers);
    read_save_data_record(filename, slot, "mission", "%d", mission);
    read_save_data_record(filename, slot, "health", "%d", &data->health);
    read_save_data_record(filename, slot, "shots", "%d", &data->shots);
    read_save_data_record(filename, slot, "reload", "%d", &data->reload);
    read_save_data_record(filename, slot, "rate", "%d", &data->rate);
    read_save_data_record(filename, slot, "ammo", "%d", &data->ammo);
    read_save_data_record(filename, slot, "gold", "%d", &data->gold);
    read_save_data_record(filename, slot, "perks", "%d", &data->perks);
    read_save_data_record(filename, slot, "xp", "%d", &data->xp);
    int salt = 0;
    read_save_data_record(filename, slot, "salt", "%d", &salt);

    data->hurts_monsters = 1;
    data->sprite = -1;
    data->alive = 1;
    data->killed = 0;

    if (get_game_settings()->require_authentication)
    {
        char read_hash_hex[DMAC_SHA1_HASH_SIZE * 2 + 1] = "";
        char saved_hash_hex[DMAC_SHA1_HASH_SIZE * 2 + 1];
        create_verification_hash(saved_hash_hex, *mission, *game_modifiers, data, salt);
        read_save_data_record(filename, slot, "hash", "%40s", read_hash_hex);
        if (strcmp(read_hash_hex, saved_hash_hex) != 0)
        {
            data->alive = 0;
            *mission = 1;
            *game_modifiers = 0;
        }
    }
}

void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers)
{
    char filename[100];
    sprintf(filename, SAVE_FILENAME, get_game_settings()->mission_pack);

    char rec[100], key[100];
    if (get_game_settings()->require_authentication)
    {
        sprintf(key, "slot_%d--hash", slot);
        if (record_file_get_record(filename, key, rec, sizeof(rec)))
        {
            *has_save = 0;
            return;
        }
    }

    sprintf(key, "slot_%d--game_modifiers", slot);
    int ret = record_file_get_record(filename, key, rec, sizeof(rec));

    if (ret)
    {
        *has_save = 0;
        return;
    }

    sscanf(rec, "%*s %d", game_modifiers);

    sprintf(key, "slot_%d--mission", slot);
    record_file_scanf(filename, key, "%*s %d", mission);

    *has_save = 1;
}

static inline void write_int_save_data(const char *filename, int slot, const char *suffix, int value)
{
    record_file_set_record_f(filename, "slot_%d--%s %d", slot, suffix, value);
}

void save_game_save_data(const char *filename, Enemy *data, int mission, int game_modifiers, int slot)
{
    write_int_save_data(filename, slot, "game_modifiers", game_modifiers);
    write_int_save_data(filename, slot, "mission", mission);
    write_int_save_data(filename, slot, "health", data->health);
    write_int_save_data(filename, slot, "shots", data->shots);
    write_int_save_data(filename, slot, "reload", data->reload);
    write_int_save_data(filename, slot, "rate", data->rate);
    write_int_save_data(filename, slot, "ammo", data->ammo);
    write_int_save_data(filename, slot, "gold", data->gold);
    write_int_save_data(filename, slot, "perks", data->perks);
    write_int_save_data(filename, slot, "xp", data->xp);

    if (get_game_settings()->require_authentication)
    {
        char hash[DMAC_SHA1_HASH_SIZE * 2 + 1];
        int salt = rand();
        create_verification_hash(hash, mission, game_modifiers, data, salt);

        record_file_set_record_f(filename, "slot_%d--hash %s", slot, hash);
        record_file_set_record_f(filename, "slot_%d--salt %d", slot, salt);
    }
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