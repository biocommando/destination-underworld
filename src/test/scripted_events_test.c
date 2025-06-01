#include <test-du.h>

#include "../bossfightconf.h"

TEST(scripted_events__pos_triggers)
{
    int p[NUM_POS_TRIGGERS];
    int expected_states[NUM_POS_TRIGGERS];
    memset(p, POS_TRIG_INIT, sizeof(p));
    memset(expected_states, POS_TRIG_INIT, sizeof(expected_states));
    // Try set invalid values... (should do nothing)
    pos_trigger_set(p, -1);
    pos_trigger_set(p, NUM_POS_TRIGGERS);

    // First, set every 3rd trigger triggered...
    for (int i = 0; i < NUM_POS_TRIGGERS; i++)
    {
        if (i % 3 == 0)
        {
            expected_states[i] = POS_TRIG_TRIGGERED;
            pos_trigger_set(p, i);
        }
        for (int j = 0; j < NUM_POS_TRIGGERS; j++)
        {
            ASSERT(INT_EQ(pos_trigger_state(p, j), expected_states[j]));
        }
    }
    // Clear the trigger values
    pos_trigger_clear(p);
    for (int i = 0; i < NUM_POS_TRIGGERS; i++)
    {
        if (expected_states[i])
            expected_states[i] = POS_TRIG_INACTIVE;
    }
    for (int j = 0; j < NUM_POS_TRIGGERS; j++)
    {
        ASSERT(INT_EQ(pos_trigger_state(p, j), expected_states[j]));
    }
    // Set all others as triggered
    for (int i = 0; i < NUM_POS_TRIGGERS; i++)
    {
        if (i % 3)
        {
            expected_states[i] = POS_TRIG_TRIGGERED;
            pos_trigger_set(p, i);
        }
        for (int j = 0; j < NUM_POS_TRIGGERS; j++)
        {
            ASSERT(INT_EQ(pos_trigger_state(p, j), expected_states[j]));
        }
    }
    // And clear them
    pos_trigger_clear(p);
    // All should be inactive
    for (int j = 0; j < NUM_POS_TRIGGERS; j++)
    {
        ASSERT(INT_EQ(pos_trigger_state(p, j), POS_TRIG_INACTIVE));
    }

    // Check invalid values
    ASSERT(INT_EQ(pos_trigger_state(p, -1), POS_TRIG_INIT));
    ASSERT(INT_EQ(pos_trigger_state(p, NUM_POS_TRIGGERS), POS_TRIG_INIT));
}