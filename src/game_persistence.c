#include <stdio.h>
#include "game_persistence.h"
#include "record_file.h"
#include "settings.h"
#include "rogue_like.h"

#include "sha1/du_dmac.h"

static inline void get_save_fn(char *dst)
{
    sprintf(dst, SAVE_FILENAME, get_game_settings()->mission_pack);
}
#define HANDLE_NULL_FN(f)       \
    char _filename[100];        \
    if (!f)                     \
    {                           \
        get_save_fn(_filename); \
        f = _filename;          \
    }

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

static inline void format_rogue_like_modifier(int slot, int idx, char *name)
{
    sprintf(name, "slot_%d_mod_%d", slot, idx);
}

static inline void format_rogue_like_flag(int slot, char *name)
{
    sprintf(name, "slot_%d_is_roguelike", slot);
}

static inline void create_rogue_like_verification_hash(char *hash_hex, const GameTuningModifier *m, int salt)
{
    char state_string[200];
    memset(state_string, 0, sizeof(state_string));
    sprintf(state_string, "amount=%lf param_id=%d salt=%d", m->amount, m->param_id, salt);
    char hash[DMAC_SHA1_HASH_SIZE];
    dmac_sha1_set_ctx(AUTH_CTX_SAVE_GAME);
    dmac_sha1_calculate_hash(hash, state_string, sizeof(state_string));
    convert_sha1_hash_to_hex(hash_hex, hash);
}

int load_rogue_like_modifiers(const char *filename, int slot, LinkedList *dst, int *gimmicks)
{
    HANDLE_NULL_FN(filename)
    linked_list_clear(dst);
    char key[32];
    format_rogue_like_flag(slot, key);
    if (record_file_find_and_read(filename, key) != 0)
        return -2;
    int auth = get_game_settings()->require_authentication;
    int idx = 0;
    while (1)
    {
        format_rogue_like_modifier(slot, idx, key);
        if (record_file_find_and_read(filename, key) != 0)
            break;
        GameTuningModifier m;
        m.amount = record_file_next_param_as_float(0);
        m.param_id = record_file_next_param_as_int(-1);
        if (idx == 0)
            *gimmicks = record_file_next_param_as_int(0);
        if (auth)
        {
            int salt = record_file_next_param_as_int(0);
            char saved_hash_hex[DMAC_SHA1_HASH_SIZE * 2 + 1];
            create_rogue_like_verification_hash(saved_hash_hex, &m, salt);
            const char *read_hash_hex = record_file_next_param();
            if (!read_hash_hex || strcmp(read_hash_hex, saved_hash_hex) != 0)
                return -1;
        }
        GameTuningModifier *m2 = LINKED_LIST_ADD(dst, GameTuningModifier);
        memcpy(m2, &m, sizeof(GameTuningModifier));
        idx++;
    }
    return 0;
}

void save_rogue_like_modifiers(const char *filename, int slot, LinkedList *src, int gimmicks)
{
    HANDLE_NULL_FN(filename)
    int auth = get_game_settings()->require_authentication;
    char key[32];
    format_rogue_like_flag(slot, key);
    record_file_find_and_modify(filename, key);
    int idx = 0;
    GameTuningModifier *m;
    LINKED_LIST_FOR_EACH(src, GameTuningModifier, m, 0)
    {
        format_rogue_like_modifier(slot, idx, key);
        record_file_find_and_modify(filename, key);
        record_file_add_float_param(m->amount);
        record_file_add_int_param(m->param_id);
        if (idx == 0)
            record_file_add_int_param(gimmicks);
        if (auth)
        {
            char hash[DMAC_SHA1_HASH_SIZE * 2 + 1];
            int salt = rand();
            create_rogue_like_verification_hash(hash, m, salt);
            record_file_add_int_param(salt);
            record_file_add_param(hash);
        }
        idx++;
    }
}

static void load_game_save_data(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot)
{
    memset(data, 0, sizeof(Enemy));

    char slot_name[20];
    format_slot_name(slot, slot_name);
    record_file_find_and_read(filename, slot_name);
    *game_modifiers = record_file_next_param_as_int(0);
    *mission = record_file_next_param_as_int(1);
    data->health = record_file_next_param_as_int(0);
    data->shots = record_file_next_param_as_int(0);
    // For now... to make it kinda backwards compatible
    data->weapon = data->shots > 1 ? 1 : 0;
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
        if (!read_hash_hex || strcmp(read_hash_hex, saved_hash_hex) != 0)
        {
            data->alive = 0;
            *mission = 1;
            *game_modifiers = 0;
        }
    }
}

void peek_into_save_data(int slot, int *has_save, int *mission, int *game_modifiers, int *is_rogue_like)
{
    *has_save = 0;
    char filename[100];
    sprintf(filename, SAVE_FILENAME, get_game_settings()->mission_pack);

    char slot_name[32];
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

    format_rogue_like_flag(slot, slot_name);
    *is_rogue_like = record_file_find_and_read(filename, slot_name) == 0;
}

static void save_game_save_data(const char *filename, Enemy *data, int mission, int game_modifiers, int slot)
{
    char slot_name[20];
    format_slot_name(slot, slot_name);
    record_file_find_and_modify(filename, slot_name);
    record_file_add_int_param(game_modifiers);
    record_file_add_int_param(mission);
    record_file_add_int_param(data->health);
    record_file_add_int_param(data->weapon + 1);
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

void save_game(const char *filename, Enemy *autosave, int mission, int game_modifiers, int slot)
{
    HANDLE_NULL_FN(filename)
    save_game_save_data(filename, autosave, mission, game_modifiers, slot);
}

void load_game(const char *filename, Enemy *data, int *mission, int *game_modifiers, int slot)
{
    HANDLE_NULL_FN(filename)
    load_game_save_data(filename, data, mission, game_modifiers, slot);
}