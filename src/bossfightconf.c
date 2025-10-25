#include "bossfightconf.h"

#include "logging.h"
#include <stdlib.h>
#include <string.h>

static void print_configs(BossFightConfig *config)
{
    LOG("Bossfight config read:\n  Events: %d\n", config->num_events);
    for (int i = 0; i < config->num_events && i < BFCONF_MAX_EVENTS; i++)
    {
        LOG("  Event %d\n    Event type: %s\n", i, bossfight_event_type_to_str(config->events[i].event_type));
        LOG("    Params: %d %d %d\n", config->events[i].parameters[0], config->events[i].parameters[1], config->events[i].parameters[2]);
        LOG("    Trigger type / value: %s = %d\n", bossfight_trigger_to_str(config->events[i].trigger_type), config->events[i].trigger_value);
        if (config->events[i].event_type == BFCONF_EVENT_TYPE_SPAWN)
        {
            LOG("    Spawn details: x %d y %d\n    ", config->events[i].spawn_point.x, config->events[i].spawn_point.y);
            for (int j = 0; j < 5; j++)
            {
                LOG(" %d: %d - %d %%", j, config->events[i].spawn_point.probability_thresholds[j][0],
                    config->events[i].spawn_point.probability_thresholds[j][1]);
            }
            LOG("\n");
        }
        if (!config->events[i].enabled)
        {
            LOG("    EVENT DISABLED\n");
        }
    }
}

static void read_property_set_cmd(int *dst, const char *buf, int game_modifiers)
{
    int val, override = -1;
    sscanf(buf, "%*s %d %d", &val, &override);
    if (override == -1 || override == game_modifiers)
        *dst = val;
}

#define BUF_LEN 100

int read_bfconfig_line(const char *buf, BossFightConfig *config, int game_modifiers, BossFightEventConfig **eventp)
{
    BossFightEventConfig *event = *eventp;
    char cmd[BUF_LEN];
    sscanf(buf, "%s", cmd);
    if (!strcmp(cmd, "health"))
    {
        read_property_set_cmd(&config->health, buf, game_modifiers);
    }
    else if (!strcmp(cmd, "speed"))
    {
        read_property_set_cmd(&config->speed, buf, game_modifiers);
    }
    else if (!strcmp(cmd, "fire_rate"))
    {
        read_property_set_cmd(&config->fire_rate, buf, game_modifiers);
    }
    else if (!strcmp(cmd, "player_initial_gold"))
    {
        read_property_set_cmd(&config->player_initial_gold, buf, game_modifiers);
    }
    else if (!strcmp(cmd, "time_starts_at"))
    {
        read_property_set_cmd(&config->state.timer_value, buf, game_modifiers);
    }
    else if (!strcmp(cmd, "event"))
    {
        if (config->num_events == BFCONF_MAX_EVENTS)
        {
            LOG("ERROR: too many events defined\n");
            return 0;
        }
        event = &config->events[config->num_events];
        event->enabled = 1;
        config->num_events++;
    }
    else if (!strcmp(cmd, "event_override"))
    {
        if (config->num_events == 0)
        {
            LOG("ERROR: overrides can only be defined after generic event\n");
            return 0;
        }
        int override_num = -1;
        sscanf(buf, "%*s %d", &override_num);
        if (override_num == game_modifiers)
        {
            event = &config->events[config->num_events - 1];
        }
        else
        {
            event = NULL;
        }
    }
    else if (!strcmp(cmd, "end"))
    {
        return 1;
    }
    else if (event)
    {
        if (!strcmp(cmd, "event_trigger"))
        {
            char trigger_type_name[BUF_LEN];
            sscanf(buf, "%*s %s %d", trigger_type_name, &event->trigger_value);
            int *trigger_type = &event->trigger_type;
            if (!strcmp(trigger_type_name, "time_interval"))
            {
                *trigger_type = BFCONF_TRIGGER_TYPE_TIME_INTERVAL;
            }
            else if (!strcmp(trigger_type_name, "time_one_time"))
            {
                *trigger_type = BFCONF_TRIGGER_TYPE_TIME_ONE_TIME;
            }
            else if (!strcmp(trigger_type_name, "health"))
            {
                *trigger_type = BFCONF_TRIGGER_TYPE_HEALTH;
            }
            else if (!strcmp(trigger_type_name, "waypoint_reached"))
            {
                *trigger_type = BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED;
            }
            else if (!strcmp(trigger_type_name, "secondary_timer"))
            {
                *trigger_type = BFCONF_TRIGGER_TYPE_SECONDARY_TIMER;
            }
            else if (!strcmp(trigger_type_name, "kill_count"))
            {
                *trigger_type = BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED;
            }
            else if (!strcmp(trigger_type_name, "positional_trigger"))
            {
                *trigger_type = BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER;
            }
        }
        else if (!strcmp(cmd, "event_action"))
        {
            char action_type[BUF_LEN] = "", str_param[BUF_LEN] = "";
            int params[7] = {0, 0, 0, 0, 0, 0, 0};
            sscanf(buf, "%*s %s %d %d %d %d %d %d %d %s", action_type, params, params + 1, params + 2,
                   params + 3, params + 4, params + 5, params + 6, str_param);
            if (!strcmp(action_type, "spawn"))
            {
                event->event_type = BFCONF_EVENT_TYPE_SPAWN;
                BossFightSpawnPointConfig *spawn_point = &event->spawn_point;
                int total = 0;
                for (int j = 0; j < 5; j++)
                {
                    int prob = params[j];
                    // >= min, < max
                    spawn_point->probability_thresholds[j][0] = j == 0 ? 0 : spawn_point->probability_thresholds[j - 1][1];
                    spawn_point->probability_thresholds[j][1] = spawn_point->probability_thresholds[j][0] + prob;
                    total += prob;
                }
                if (total > 100)
                {
                    LOG_ERROR("Spawn point probability sum over 100: %d\n", total);
                }
                spawn_point->x = params[5];
                spawn_point->y = params[6];
            }
            else if (!strcmp(action_type, "allow_firing"))
            {
                event->event_type = BFCONF_EVENT_TYPE_ALLOW_FIRING;
            }
            else if (!strcmp(action_type, "disallow_firing"))
            {
                event->event_type = BFCONF_EVENT_TYPE_DISALLOW_FIRING;
            }
            else if (!strcmp(action_type, "fire_in_circle"))
            {
                event->event_type = BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE;
                event->parameters[0] = params[0]; // number_of_directions
                event->parameters[1] = params[1]; // intensity
            }
            else if (!strcmp(action_type, "modify_terrain"))
            {
                event->event_type = BFCONF_EVENT_TYPE_MODIFY_TERRAIN;
                event->parameters[0] = params[0]; // x
                event->parameters[1] = params[1]; // y
                if (!strcmp(str_param, "floor"))
                {
                    event->parameters[2] = BFCONF_MODIFY_TERRAIN_FLOOR;
                }
                if (!strcmp(str_param, "wall"))
                {
                    event->parameters[2] = BFCONF_MODIFY_TERRAIN_WALL;
                }
                if (!strcmp(str_param, "level_exit"))
                {
                    event->parameters[2] = BFCONF_MODIFY_TERRAIN_EXIT;
                }
            }
            else if (!strcmp(action_type, "set_waypoint"))
            {
                event->event_type = BFCONF_EVENT_TYPE_SET_WAYPOINT;
                event->parameters[0] = params[0]; // x
                event->parameters[1] = params[1]; // y
                event->parameters[2] = params[2]; // waypoint_id
            }
            else if (!strcmp(action_type, "clear_waypoint"))
            {
                event->event_type = BFCONF_EVENT_TYPE_CLEAR_WAYPOINT;
            }
            else if (!strcmp(action_type, "start_secondary_timer"))
            {
                event->event_type = BFCONF_EVENT_TYPE_START_SECONDARY_TIMER;
                event->parameters[0] = params[0]; // time
            }
            else if (!strcmp(action_type, "stop_secondary_timer"))
            {
                event->event_type = BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER;
            }
            else if (!strcmp(action_type, "toggle_event_enabled"))
            {
                event->event_type = BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED;
                int event_id = params[0];
                if (event_id < 0) //|| event_id >= config->num_events)
                {
                    LOG("Invalid event_id field %d\n", event_id);
                    event_id = 0;
                }
                event->parameters[0] = event_id;
                event->parameters[1] = params[1]; // enabled
            }
            else if (!strcmp(action_type, "spawn_potion"))
            {
                event->event_type = BFCONF_EVENT_TYPE_SPAWN_POTION;
                event->parameters[0] = params[0]; // x
                event->parameters[1] = params[1]; // y
                event->parameters[2] = params[2]; // type
            }
        }
        else if (!strcmp(cmd, "event_initially_disabled"))
        {
            event->enabled = 0;
        }
    }
    *eventp = event;
    return 0;
}

void bfconfig_init(BossFightConfig *config)
{
    memset(config, 0, sizeof(BossFightConfig));
    config->player_initial_gold = -1;

    // Disable waypoints by default;
    // otherwise boss will try to go to coordinate 0,0
    config->state.boss_waypoint.x = -1;
    config->state.boss_waypoint.y = -1;
}

void bfconfig_finalize(BossFightConfig *config)
{
    config->state.previous_health = config->health + 1;
    print_configs(config);
}

void read_bfconfig(FILE *f, BossFightConfig *config, int game_modifiers)
{
    bfconfig_init(config);

    char buf[BUF_LEN];
    BossFightEventConfig *event = NULL;
    while (!feof(f))
    {
        fgets(buf, sizeof(buf), f);
        if (read_bfconfig_line(buf, config, game_modifiers, &event) != 0)
            break;
    }

    bfconfig_finalize(config);
}

void bossfight_process_event_triggers(BossFightConfig *config)
{
    BossFightState *state = &config->state;
    if (state->secondary_timer_started)
    {
        state->secondary_timer_value++;
    }
    state->timer_value++;
    int tv = state->timer_value;
    for (int i = 0; i < config->num_events; i++)
    {
        BossFightEventConfig *econf = &config->events[i];
        int *trig = &state->triggers[i];
        switch (econf->trigger_type)
        {
        case BFCONF_TRIGGER_TYPE_TIME_INTERVAL:
            *trig = (tv % econf->trigger_value) == 0;
            break;
        case BFCONF_TRIGGER_TYPE_TIME_ONE_TIME:
            *trig = tv == econf->trigger_value;
            break;
        case BFCONF_TRIGGER_TYPE_HEALTH:
            *trig = state->health <= econf->trigger_value && state->previous_health > econf->trigger_value;
            break;
        case BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED:
            *trig = state->waypoint_reached && econf->trigger_value == state->waypoint;
            break;
        case BFCONF_TRIGGER_TYPE_SECONDARY_TIMER:
            *trig = state->secondary_timer_started && state->secondary_timer_value == econf->trigger_value;
            break;
        case BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED:
            *trig = state->player_kills >= econf->trigger_value && state->player_previous_kills < econf->trigger_value;
            break;
        case BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER:
            *trig = pos_trigger_state(state->positional_trigger_flags, econf->trigger_value) == POS_TRIG_TRIGGERED;
            break;
        default:
            *trig = 0;
            break;
        }
    }
    state->player_previous_kills = state->player_kills;
    state->previous_health = state->health;
    state->waypoint_reached = 0;
}

const char *bossfight_trigger_to_str(int value)
{
    switch (value)
    {
    case BFCONF_TRIGGER_TYPE_TIME_INTERVAL:
        return "Time interval";
    case BFCONF_TRIGGER_TYPE_TIME_ONE_TIME:
        return "Time one time";
    case BFCONF_TRIGGER_TYPE_HEALTH:
        return "Health";
    case BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED:
        return "Waypoint reached";
    case BFCONF_TRIGGER_TYPE_SECONDARY_TIMER:
        return "Secondary timer";
    case BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED:
        return "Kill count reached";
    case BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER:
        return "positional trigger";
    case BFCONF_TRIGGER_TYPE_NEVER:
        return "Never";
    default:
        return "";
    }
}

const char *bossfight_event_type_to_str(int value)
{
    switch (value)
    {
    case BFCONF_EVENT_TYPE_NO_OP:
        return "No-op";
    case BFCONF_EVENT_TYPE_SPAWN:
        return "Spawn";
    case BFCONF_EVENT_TYPE_ALLOW_FIRING:
        return "Allow firing";
    case BFCONF_EVENT_TYPE_DISALLOW_FIRING:
        return "Disallow firing";
    case BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE:
        return "Fire in circle";
    case BFCONF_EVENT_TYPE_MODIFY_TERRAIN:
        return "Modify terrain";
    case BFCONF_EVENT_TYPE_SET_WAYPOINT:
        return "Set waypoint";
    case BFCONF_EVENT_TYPE_CLEAR_WAYPOINT:
        return "Clear waypoint";
    case BFCONF_EVENT_TYPE_START_SECONDARY_TIMER:
        return "Start secondary timer";
    case BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER:
        return "Stop secondary timer";
    case BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED:
        return "Toggle event enabled";

    default:
        return "";
    }
}

int pos_trigger_state(const int *positional_trigger_flags, int ptrig_idx)
{
    if (ptrig_idx < 0 || ptrig_idx >= NUM_POS_TRIGGERS)
        return 0;
    return positional_trigger_flags[ptrig_idx];
}

void pos_trigger_clear(int *positional_trigger_flags)
{
    for (int i = 0; i < NUM_POS_TRIGGERS; i++)
    {
        if (positional_trigger_flags[i])
            positional_trigger_flags[i] = POS_TRIG_INACTIVE;
    }
}

void pos_trigger_set(int *positional_trigger_flags, int ptrig_idx)
{
    if (ptrig_idx < 0 || ptrig_idx >= NUM_POS_TRIGGERS)
        return;
    LOG_TRACE("pos trigger current state %x\n", *positional_trigger_flags);
    if (positional_trigger_flags[ptrig_idx] == POS_TRIG_INIT)
        positional_trigger_flags[ptrig_idx] = POS_TRIG_TRIGGERED;
    LOG_TRACE("pos trigger, new state %x\n", *positional_trigger_flags);
}