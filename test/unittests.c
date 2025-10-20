#include <unittests.h>

int logging_enabled = 0;

TEST_MAIN(all_tests)

void all_tests()
{
    RUN_TEST_SUITE(test_suite__synth)
    RUN_TEST_SUITE(test_suite__record_file)
    RUN_TEST_SUITE(test_suite__variables)
    RUN_TEST_SUITE(test_suite__arenaconf)
    RUN_TEST_SUITE(test_suite__scripted_events)
    RUN_TEST_SUITE(test_suite__linked_list)
    RUN_TEST_SUITE(test_suite__command_file)
}