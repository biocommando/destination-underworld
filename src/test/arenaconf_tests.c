#include <unittests.h>
#include <stdio.h>
#include <string.h>
#include "../arenaconf.h"
#include "../record_file.h"
#include "../settings.h"
#include "../duConstants.h"

TEST(arenaconf__read_arena_configs__valid_and_invalid_entries)
{
    // The directory doesn't exist so the test records don't get written to disk
    const char *testfile = "path/does/not/exist";

    ArenaConfigs ac;

    record_file_set_record(testfile, "number_of_arenas", "number_of_arenas 5");
    record_file_set_record(testfile, "arena_0", "arena_0 level_number=123 name=A fancy name");
    // Arena 1 missing

    // Name missing
    record_file_set_record(testfile, "arena_2", "arena_2 level_number=321");

    char long_name[100];
    memset(long_name, 0, sizeof(long_name));
    memset(long_name, 'A', sizeof(ac.arenas[0].name) - 1);
    // Arena name with max characters
    record_file_set_record_f(testfile, "arena_3 level_number=555 name=%s", long_name);
    long_name[sizeof(ac.arenas[0].name) - 1] = 'A';
    // Too long arena name
    record_file_set_record_f(testfile, "arena_4 level_number=333 name=%s", long_name);

    read_arena_configs(testfile, &ac);

    record_file_flush();

    ASSERT(INT_EQ(ac.number_of_arenas, 5));
    
    ASSERT(INT_EQ(ac.arenas[0].level_number, 123));
    ASSERT(STR_EQ(ac.arenas[0].name, "A fancy name"));

    // Reading failed -> skip the entry
    ASSERT(INT_EQ(ac.arenas[1].level_number, 0));
    ASSERT(STR_EQ(ac.arenas[1].name, ""));

    ASSERT(INT_EQ(ac.arenas[2].level_number, 321));
    ASSERT(STR_EQ(ac.arenas[2].name, "Arena level 3"));

    long_name[sizeof(ac.arenas[0].name) - 1] = 0;
    ASSERT(INT_EQ(ac.arenas[3].level_number, 555));
    ASSERT(STR_EQ(ac.arenas[3].name, long_name));
    
    ASSERT(INT_EQ(ac.arenas[4].level_number, 333));
    ASSERT(STR_EQ(ac.arenas[4].name, "Arena level 5"));
}

TEST(arenaconf__get_arena_highscore)
{
    // The directory doesn't exist so the test records don't get written to disk
    strcpy(get_game_settings()->mission_pack, "not-real-missionpack");

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

TEST(arenaconf__set_arena_highscore)
{
    // The directory doesn't exist so the test records don't get written to disk
    strcpy(get_game_settings()->mission_pack, "not-real-missionpack");

    const char *testfile = DATADIR "not-real-missionpack/arcade_mode_highscores.dat";

    set_arena_highscore(123, 1, 10);
    ASSERT(INT_EQ(get_arena_highscore(123, 1), 10));
    set_arena_highscore(123, 1 + 32, 15);
    ASSERT(INT_EQ(get_arena_highscore(123, 1), 15));
    set_arena_highscore(123, 0, 10);
    ASSERT(INT_EQ(get_arena_highscore(123, 0), 10));

    record_file_flush();
}