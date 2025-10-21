#include <unittests.h>
#include <stdio.h>
#include <string.h>
#include "../arenaconf.h"
#include "../record_file.h"
#include "../settings.h"
#include "../duConstants.h"

void arenaconf__read_arena_configs__valid_and_invalid_entries()
{
    ArenaConfigs ac;
    const char *testfile = "testfile.txt";
    FILE *f = fopen(testfile, "w");
    fprintf(f, "add:level_number=\"123\" name=\"A fancy name\"\n");
    // Name missing
    fprintf(f, "add:level_number=\"321\"\n");

    char long_name[100];
    memset(long_name, 0, sizeof(long_name));
    memset(long_name, 'A', sizeof(ac.arenas[0].name) - 1);
    // Arena name with max characters
    fprintf(f, "add:level_number=\"555\" name=\"%s\"\n", long_name);
    long_name[sizeof(ac.arenas[0].name) - 1] = 'A';
    // Too long arena name
    fprintf(f, "add:level_number=\"333\" name=\"%s\"\n", long_name);
    record_file_set_record_f(testfile, "arena_4 level_number=333 name=%s", long_name);
    fprintf(f, "add:level_number=\"666\" name=\"One more\"\n");
    fclose(f);

    read_arena_configs(testfile, &ac);

    remove(testfile);

    ASSERT(INT_EQ(ac.number_of_arenas, 3));

    ASSERT(INT_EQ(ac.arenas[0].level_number, 123));
    ASSERT(STR_EQ(ac.arenas[0].name, "A fancy name"));

    long_name[sizeof(ac.arenas[0].name) - 1] = 0;
    ASSERT(INT_EQ(ac.arenas[1].level_number, 555));
    ASSERT(STR_EQ(ac.arenas[1].name, long_name));

    ASSERT(INT_EQ(ac.arenas[2].level_number, 666));
    ASSERT(STR_EQ(ac.arenas[2].name, "One more"));
}

void arenaconf__get_arena_highscore()
{
    const char *testfile = DATADIR "not-real-missionpack/arcade_mode_highscores.dat";

    record_file_set_record(testfile, "level_number=123;mode=1;", "level_number=123;mode=1; 1");
    record_file_set_record(testfile, "level_number=321;mode=1;", "level_number=321;mode=1; 2");
    record_file_set_record(testfile, "level_number=123;mode=2;", "level_number=123;mode=2; 3");
    record_file_set_record(testfile, "level_number=321;mode=2;", "level_number=321;mode=2; 4");

    ASSERT(INT_EQ(get_arena_highscore(123, 1), 1));
    ASSERT(INT_EQ(get_arena_highscore(321, 1), 2));
    ASSERT(INT_EQ(get_arena_highscore(123, 2), 3));
    ASSERT(INT_EQ(get_arena_highscore(321, 2), 4));
    // Arena fight modifier should not effect read result
    ASSERT(INT_EQ(get_arena_highscore(321, 2 + 32), 4));
    // Entry not found
    ASSERT(INT_EQ(get_arena_highscore(321, 3), 0));
    ASSERT(INT_EQ(get_arena_highscore(4321, 1), 0));
    record_file_flush();
}

void arenaconf__set_arena_highscore()
{
    const char *testfile = DATADIR "not-real-missionpack/arcade_mode_highscores.dat";

    set_arena_highscore(123, 1, 10);
    ASSERT(INT_EQ(get_arena_highscore(123, 1), 10));
    set_arena_highscore(123, 1 + 32, 15);
    ASSERT(INT_EQ(get_arena_highscore(123, 1), 15));
    set_arena_highscore(123, 0, 10);
    ASSERT(INT_EQ(get_arena_highscore(123, 0), 10));

    record_file_flush();
}

void test_suite__arenaconf()
{
    char orig_mission_pack[64];
    memcpy(orig_mission_pack, get_game_settings()->mission_pack, sizeof(orig_mission_pack));

    // The directory doesn't exist so the test records don't get written to disk
    strcpy(get_game_settings()->mission_pack, "not-real-missionpack");

    RUN_TEST(arenaconf__read_arena_configs__valid_and_invalid_entries);
    RUN_TEST(arenaconf__get_arena_highscore);
    RUN_TEST(arenaconf__set_arena_highscore);

    memcpy(get_game_settings()->mission_pack, orig_mission_pack, sizeof(orig_mission_pack));
}
