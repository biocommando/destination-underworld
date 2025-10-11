#include <unittests.h>

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

TEST(scripted_events__read_bfconfig)
{
    const char *fname = "scripted_events__read_bfconfig.txt";
    FILE *f = fopen(fname, "w");
    fprintf(f, "xxx" // Will be skipped when reading config
               "health 1\n"
               "speed 2\n"
               "fire_rate 3\n"
               "player_initial_gold 4\n"
               "time_starts_at 5\n"
               "health 11 1\n"
               "speed 22 1\n"
               "fire_rate 33 1\n"
               "player_initial_gold 44 1\n"
               "time_starts_at 55 1\n"
               "event\n"
               "event_trigger time_interval 10\n"
               "event_action spawn " /* probabilities = */ "10 20 30 15 25 " /* x, y = */ "11 12\n"
               "event_override 1\n"
               "event_trigger time_one_time 20\n"
               "event\n"
               "event_trigger health 30\n"
               "event_action disallow_firing\n"
               "event_override 1\n"
               "event_action allow_firing\n"
               "event\n"
               "event_trigger waypoint_reached 40\n"
               "event_action modify_terrain 1 2 0 0 0 0 0 floor\n"
               "event\n"
               "event_trigger secondary_timer 50\n"
               "event_action modify_terrain 2 3 0 0 0 0 0 wall\n"
               "event\n"
               "event_trigger kill_count 60\n"
               "event_action modify_terrain 3 4 0 0 0 0 0 level_exit\n"
               "event_override 1\n"
               "event_initially_disabled\n"
               "event\n"
               "event_trigger positional_trigger 70\n"
               "event_action set_waypoint 5 6 123\n"
               "event\n"
               "event_trigger time_interval 10\n"
               "event_action clear_waypoint\n"
               "event\n"
               "event_trigger time_interval 10\n"
               "event_action start_secondary_timer 321\n"
               "event\n"
               "event_trigger time_interval 10\n"
               "event_action stop_secondary_timer\n"
               "event_initially_disabled\n"
               "event\n"
               "event_trigger time_interval 10\n"
               "event_action toggle_event_enabled 5 1\n"
               "event\n"
               "event_trigger time_interval 10\n"
               "event_action spawn_potion 7 77 777\n"
               "event\n"
               "event_trigger never 0\n"
               "event_action fire_in_circle 8 2\n"
               "end\n"
               // Extra stuff after end shouldn't matter
               "event\n"
               "event_trigger time_interval 10\n"
               "event_action toggle_event_enabled 5 1\n");
    fclose(f);

#define CHECK_EVENT(enabled_, trigger_type_, trigger_value_, event_type_, parameter0, parameter1, parameter2) \
    ASSERT(INT_EQ(ev->enabled, enabled_));                                                                    \
    ASSERT(INT_EQ(ev->trigger_type, trigger_type_));                                                          \
    ASSERT(INT_EQ(ev->trigger_value, trigger_value_));                                                        \
    ASSERT(INT_EQ(ev->event_type, event_type_));                                                              \
    ASSERT(INT_EQ(ev->parameters[0], parameter0));                                                            \
    ASSERT(INT_EQ(ev->parameters[1], parameter1));                                                            \
    ASSERT(INT_EQ(ev->parameters[2], parameter2));                                                            \
    ev++;

    f = fopen(fname, "r");
    fseek(f, 3, SEEK_SET);
    BossFightConfig c;
    read_bfconfig(f, &c, 777);
    ASSERT(INT_EQ(c.health, 1));
    ASSERT(INT_EQ(c.speed, 2));
    ASSERT(INT_EQ(c.fire_rate, 3));
    ASSERT(INT_EQ(c.player_initial_gold, 4));
    ASSERT(INT_EQ(c.state.timer_value, 5));
    ASSERT(INT_EQ(c.state.previous_health, 2));

    ASSERT(INT_EQ(c.num_events, 12));

    const BossFightEventConfig *ev = c.events;
    ASSERT(INT_EQ(ev->spawn_point.x, 11));
    ASSERT(INT_EQ(ev->spawn_point.y, 12));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[0][0], 0));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[0][1], 10));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[1][0], 10));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[1][1], 30));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[2][0], 30));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[2][1], 60));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[3][0], 60));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[3][1], 75));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[4][0], 75));
    ASSERT(INT_EQ(ev->spawn_point.probability_thresholds[4][1], 100));

    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_SPAWN, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_HEALTH, 30, BFCONF_EVENT_TYPE_DISALLOW_FIRING, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED, 40, BFCONF_EVENT_TYPE_MODIFY_TERRAIN, 1, 2, BFCONF_MODIFY_TERRAIN_FLOOR)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_SECONDARY_TIMER, 50, BFCONF_EVENT_TYPE_MODIFY_TERRAIN, 2, 3, BFCONF_MODIFY_TERRAIN_WALL)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED, 60, BFCONF_EVENT_TYPE_MODIFY_TERRAIN, 3, 4, BFCONF_MODIFY_TERRAIN_EXIT)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER, 70, BFCONF_EVENT_TYPE_SET_WAYPOINT, 5, 6, 123)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_CLEAR_WAYPOINT, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_START_SECONDARY_TIMER, 321, 0, 0)
    CHECK_EVENT(0, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED, 5, 1, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_SPAWN_POTION, 7, 77, 777)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_NEVER, 0, BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE, 8, 2, 0)

    fseek(f, 3, SEEK_SET);
    read_bfconfig(f, &c, 1);
    fclose(f);
    ASSERT(INT_EQ(c.health, 11));
    ASSERT(INT_EQ(c.speed, 22));
    ASSERT(INT_EQ(c.fire_rate, 33));
    ASSERT(INT_EQ(c.player_initial_gold, 44));
    ASSERT(INT_EQ(c.state.timer_value, 55));
    ASSERT(INT_EQ(c.state.previous_health, 12));

    ASSERT(INT_EQ(c.num_events, 12));
    ev = c.events;
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_ONE_TIME, 20, BFCONF_EVENT_TYPE_SPAWN, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_HEALTH, 30, BFCONF_EVENT_TYPE_ALLOW_FIRING, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED, 40, BFCONF_EVENT_TYPE_MODIFY_TERRAIN, 1, 2, BFCONF_MODIFY_TERRAIN_FLOOR)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_SECONDARY_TIMER, 50, BFCONF_EVENT_TYPE_MODIFY_TERRAIN, 2, 3, BFCONF_MODIFY_TERRAIN_WALL)
    CHECK_EVENT(0, BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED, 60, BFCONF_EVENT_TYPE_MODIFY_TERRAIN, 3, 4, BFCONF_MODIFY_TERRAIN_EXIT)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER, 70, BFCONF_EVENT_TYPE_SET_WAYPOINT, 5, 6, 123)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_CLEAR_WAYPOINT, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_START_SECONDARY_TIMER, 321, 0, 0)
    CHECK_EVENT(0, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER, 0, 0, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED, 5, 1, 0)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 10, BFCONF_EVENT_TYPE_SPAWN_POTION, 7, 77, 777)
    CHECK_EVENT(1, BFCONF_TRIGGER_TYPE_NEVER, 0, BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE, 8, 2, 0)

    remove(fname);
}

#define ADD_EVENT_TRIGGER(trigger_type_, trigger_value_) \
    ev->trigger_type = trigger_type_;                    \
    ev->trigger_value = trigger_value_;                  \
    ev->enabled = 1;                                     \
    ev++;                                                \
    c.num_events++;

TEST(scripted_events__bossfight_process_event_triggers__BFCONF_TRIGGER_TYPE_TIME_INTERVAL)
{
    BossFightConfig c;
    memset(&c, 0, sizeof(c));
    BossFightEventConfig *ev = c.events;
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 2);
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_TIME_INTERVAL, 3);
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 1));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    ASSERT(INT_EQ(c.state.triggers[1], 1));
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 1));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
}

TEST(scripted_events__bossfight_process_event_triggers__BFCONF_TRIGGER_TYPE_TIME_ONE_TIME)
{
    BossFightConfig c;
    memset(&c, 0, sizeof(c));
    BossFightEventConfig *ev = c.events;
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_TIME_ONE_TIME, 2);
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 1));
    for (int i = 0; i < 10; i++)
    {
        bossfight_process_event_triggers(&c);
        ASSERT(INT_EQ(c.state.triggers[0], 0));
    }
}

TEST(scripted_events__bossfight_process_event_triggers__BFCONF_TRIGGER_TYPE_HEALTH)
{
    BossFightConfig c;
    memset(&c, 0, sizeof(c));
    BossFightEventConfig *ev = c.events;
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_HEALTH, 10);
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_HEALTH, 20);
    c.health = 20;
    c.state.previous_health = 21;
    c.state.health = c.health;
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    ASSERT(INT_EQ(c.state.triggers[1], 1));
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
    c.state.health = 11;
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
    c.state.health = 10;
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 1));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
    c.state.health = 5;
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    ASSERT(INT_EQ(c.state.triggers[1], 0));
}

TEST(scripted_events__bossfight_process_event_triggers__BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED)
{
    BossFightConfig c;
    memset(&c, 0, sizeof(c));
    BossFightEventConfig *ev = c.events;
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED, 123);
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    c.state.waypoint_reached = 1;
    c.state.waypoint = 321;
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.waypoint_reached, 0));
    ASSERT(INT_EQ(c.state.triggers[0], 0));
    c.state.waypoint_reached = 1;
    c.state.waypoint = 123;
    bossfight_process_event_triggers(&c);
    ASSERT(INT_EQ(c.state.triggers[0], 1));
}

TEST(scripted_events__bossfight_process_event_triggers__BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED)
{
    BossFightConfig c;
    memset(&c, 0, sizeof(c));
    BossFightEventConfig *ev = c.events;
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED, 10);
    for (int k = 0; k < 20; k++)
    {
        bossfight_process_event_triggers(&c);
        ASSERT(INT_EQ(c.state.triggers[0], k == 10));
        c.state.player_kills++;
    }
}

TEST(scripted_events__bossfight_process_event_triggers__BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER)
{
    BossFightConfig c;
    memset(&c, 0, sizeof(c));
    BossFightEventConfig *ev = c.events;
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER, 1);
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER, 5);
    ADD_EVENT_TRIGGER(BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER, 10);
    for (int i = 0; i < NUM_POS_TRIGGERS; i++)
    {
        pos_trigger_set(c.state.positional_trigger_flags, i);
        bossfight_process_event_triggers(&c);
        ASSERT(INT_EQ(c.state.triggers[0], i == 1));
        ASSERT(INT_EQ(c.state.triggers[1], i == 5));
        ASSERT(INT_EQ(c.state.triggers[2], i == 10));

        pos_trigger_clear(c.state.positional_trigger_flags);
    }
}