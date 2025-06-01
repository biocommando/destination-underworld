#include <test-du.h>

#include "../bossfightconf.h"

TEST(scripted_events__pos_triggers)
{
    int p = 0;
    int expected_states[16];
    memset(expected_states, 0, sizeof(expected_states));
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            ASSERT(INT_EQ(pos_trigger_state(&p, j), expected_states[j]));
        }
        expected_states[i * 3] = 1;
        pos_trigger_set(&p, i * 3);
    }
    pos_trigger_clear(&p);
    for (int i = 0; i < 16; i++)
    {
        if (expected_states[i])
            expected_states[i] = -1;
    }
    for (int j = 0; j < 16; j++)
    {
        ASSERT(INT_EQ(pos_trigger_state(&p, j), expected_states[j]));
    }
}