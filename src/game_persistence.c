#include <stdio.h>
#include "game_persistence.h"
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
    dmac_sha1_set_ctx(AUTH_CTX_SAVE_GAME);
    dmac_sha1_calculate_hash(hash, state_string, sizeof(state_string));
    convert_sha1_hash_to_hex(hash_hex, hash);
}

static inline void format_slot_name(int slot, char *name)
{
    sprintf(name, "slot_%d", slot);
}

void load_game_save_data(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot)
{
    memset(data, 0, sizeof(Enemy));

    char slot_name[20];
    format_slot_name(slot, slot_name);
    record_file_find_and_read(filename, slot_name);
    *game_modifiers = record_file_next_param_as_int(0);
    *mission = record_file_next_param_as_int(1);
    data->health = record_file_next_param_as_int(0);
    data->shots = record_file_next_param_as_int(0);
    data->reload = record_file_next_param_as_int(0);
    data->rate = record_file_next_param_as_int(0);
    data->ammo = record_file_next_param_as_int(0);
    data->gold = record_file_next_param_as_int(0);
    data->perks = record_file_next_param_as_int(0);
    data->xp = record_file_next_param_as_int(0);
    int salt = record_file_next_param_as_int(0);

    data->hurts_monsters = 1;
    data->sprite = -1;
    data->alive = 1;
    data->killed = 0;

    if (get_game_settings()->require_authentication)
    {
        char saved_hash_hex[DMAC_SHA1_HASH_SIZE * 2 + 1];
        create_verification_hash(saved_hash_hex, *mission, *game_modifiers, data, salt);
        const char *read_hash_hex = record_file_next_param();
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
    *has_save = 0;
    char filename[100];
    sprintf(filename, SAVE_FILENAME, get_game_settings()->mission_pack);

    char slot_name[20];
    format_slot_name(slot, slot_name);
    if (record_file_find_and_read(filename, slot_name) != 0)
    {
        return;
    }
    int _game_modifiers = record_file_next_param_as_int(-1);
    int _mission = record_file_next_param_as_int(-1);

    if (_game_modifiers < 0 || _mission < 0)
    {
        return;
    }
    for (int i = 0; i < 9; i++)
    {
        record_file_next_param();
    }

    if (get_game_settings()->require_authentication && !record_file_next_param())
    {
        return;
    }

    *game_modifiers = _game_modifiers;
    *mission = _mission;

    *has_save = 1;
}

void save_game_save_data(const char *filename, Enemy *data, int mission, int game_modifiers, int slot)
{
    char slot_name[20];
    format_slot_name(slot, slot_name);
    record_file_find_and_modify(filename, slot_name);
    record_file_add_int_param(game_modifiers);
    record_file_add_int_param(mission);
    record_file_add_int_param(data->health);
    record_file_add_int_param(data->shots);
    record_file_add_int_param(data->reload);
    record_file_add_int_param(data->rate);
    record_file_add_int_param(data->ammo);
    record_file_add_int_param(data->gold);
    record_file_add_int_param(data->perks);
    record_file_add_int_param(data->xp);

    if (get_game_settings()->require_authentication)
    {
        char hash[DMAC_SHA1_HASH_SIZE * 2 + 1];
        int salt = rand();
        create_verification_hash(hash, mission, game_modifiers, data, salt);

        record_file_add_int_param(salt);
        record_file_add_param(hash);
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