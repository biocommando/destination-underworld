// Compile with:
// gcc .\src\add_save_data_hash.c .\src\record_file.c .\src\sha1\sha1.c .\src\sha1\du_dmac.c

#include "sha1/du_dmac.h"
#include "record_file.h"
#include <stdio.h>
#include <string.h>

#define READ_SAVE_DATA_RECORD(data) \
    int data = record_file_next_param_as_int(0)

int main(int argc, char **argv)
{
    dmac_sha1_set_ctx(2);
    if (argc < 2)
    {
        printf("Save data filename required\n");
        return 1;
    }

    const char *filename = argv[1];
    for (int slot = 0; slot < 9; slot++)
    {
        char slot_name[20];
        sprintf(slot_name, "slot_%d", slot);
        record_file_find_and_read(filename, slot_name);
        READ_SAVE_DATA_RECORD(game_modifiers);
        READ_SAVE_DATA_RECORD(mission);
        READ_SAVE_DATA_RECORD(health);
        READ_SAVE_DATA_RECORD(shots);
        READ_SAVE_DATA_RECORD(reload);
        READ_SAVE_DATA_RECORD(rate);
        READ_SAVE_DATA_RECORD(ammo);
        READ_SAVE_DATA_RECORD(gold);
        READ_SAVE_DATA_RECORD(perks);
        READ_SAVE_DATA_RECORD(xp);
        READ_SAVE_DATA_RECORD(salt);
        char hash_hex[DMAC_SHA1_HASH_SIZE * 2 + 1] = "";
        char state_string[200];
        memset(state_string, 0, sizeof(state_string));
        sprintf(state_string, "game data %d %d %d %d %d %d %d %d %d %d salt=%d",
                game_modifiers, mission,
                health, shots, reload,
                rate, ammo, gold, perks,
                xp, salt);
        char hash[DMAC_SHA1_HASH_SIZE];
        dmac_sha1_calculate_hash(hash, state_string, sizeof(state_string));
        convert_sha1_hash_to_hex(hash_hex, hash);

        record_file_find_and_modify(filename, slot_name);
        record_file_add_int_param(game_modifiers);
        record_file_add_int_param(mission);
        record_file_add_int_param(health);
        record_file_add_int_param(shots);
        record_file_add_int_param(reload);
        record_file_add_int_param(rate);
        record_file_add_int_param(ammo);
        record_file_add_int_param(gold);
        record_file_add_int_param(perks);
        record_file_add_int_param(xp);
        record_file_add_int_param(salt);
        record_file_add_param(hash_hex);
    }
    record_file_flush();
    return 0;
}