#include "bossfightconf.h"

#include "logging.h"
#include <stdlib.h>
#include <string.h>

static void print_configs(BossFightConfig *config)
{
  char s[100];
  LOG("Bossfight config read:\n  Events: %d\n", config->num_events);
  for (int i = 0; i < config->num_events && i < BFCONF_MAX_EVENTS; i++)
  {
    bossfight_event_type_to_str(s, config->events[i].event_type);
    LOG("  Event %d\n    Event type: %s\n", i, s);
    LOG("    Params: %d %d %d\n", config->events[i].parameters[0], config->events[i].parameters[1], config->events[i].parameters[2]);
    bossfight_trigger_to_str(s, config->events[i].trigger_type);
    LOG("    Trigger type / value: %s = %d\n", s, config->events[i].trigger_value);
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

void read_bfconfig_new(FILE *f, BossFightConfig *config, int game_modifiers)
{
  memset(config, 0, sizeof(BossFightConfig));
  config->player_initial_gold = -1;

  // Disable waypoints by default;
  // otherwise boss will try to go to coordinate 0,0
  config->state.boss_waypoint.x = -1;
  config->state.boss_waypoint.y = -1;

  char buf[100];
  BossFightEventConfig *event = NULL;
  while (!feof(f))
  {
    fgets(buf, sizeof(buf), f);
    char cmd[100];
    sscanf(buf, "%s", cmd);
    if (!strcmp(cmd, "health"))
    {
      int val, override = -1;
      sscanf(buf, "%*s %d %d", &val, &override);
      if (override == -1 || override == game_modifiers)
        config->health = val;
    }
    else if (!strcmp(cmd, "speed"))
    {
      int val, override = -1;
      sscanf(buf, "%*s %d %d", &val, &override);
      if (override == -1 || override == game_modifiers)
        config->speed = val;
    }
    else if (!strcmp(cmd, "fire_rate"))
    {
      int val, override = -1;
      sscanf(buf, "%*s %d %d", &val, &override);
      if (override == -1 || override == game_modifiers)
        config->fire_rate = val;
    }
    else if (!strcmp(cmd, "player_initial_gold"))
    {
      int val, override = -1;
      sscanf(buf, "%*s %d %d", &val, &override);
      if (override == -1 || override == game_modifiers)
        config->player_initial_gold = val;
    }
    else if (!strcmp(cmd, "time_starts_at"))
    {
      int val, override = -1;
      sscanf(buf, "%*s %d %d", &val, &override);
      if (override == -1 || override == game_modifiers)
        config->state.timer_value = val;
    }
    else if (!strcmp(cmd, "event"))
    {
      if (config->num_events == BFCONF_MAX_EVENTS)
      {
        LOG("ERROR: too many events defined\n");
        continue;
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
        continue;
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
      break;
    }
    else if (event)
    {
      if (!strcmp(cmd, "event_trigger"))
      {
        char s[100];
        sscanf(buf, "%*s %s %d", s, &event->trigger_value);
        int *trigger_type = &event->trigger_type;
        if (!strcmp(s, "time_interval"))
        {
          *trigger_type = BFCONF_TRIGGER_TYPE_TIME_INTERVAL;
        }
        if (!strcmp(s, "time_one_time"))
        {
          *trigger_type = BFCONF_TRIGGER_TYPE_TIME_ONE_TIME;
        }
        if (!strcmp(s, "health"))
        {
          *trigger_type = BFCONF_TRIGGER_TYPE_HEALTH;
        }
        if (!strcmp(s, "waypoint_reached"))
        {
          *trigger_type = BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED;
        }
        if (!strcmp(s, "secondary_timer"))
        {
          *trigger_type = BFCONF_TRIGGER_TYPE_SECONDARY_TIMER;
        }
        if (!strcmp(s, "kill_count"))
        {
          *trigger_type = BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED;
        }
        if (!strcmp(s, "positional_trigger"))
        {
          *trigger_type = BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER;
        }
      }
      else if (!strcmp(cmd, "event_action"))
      {
        char s[100] = "", s2[100] = "";
        int params[7] = {0, 0, 0, 0, 0, 0, 0};
        sscanf(buf, "%*s %s %d %d %d %d %d %d %d %s", s, params, params + 1, params + 2,
               params + 3, params + 4, params + 5, params + 6, s2);
        if (!strcmp(s, "spawn"))
        {
          event->event_type = BFCONF_EVENT_TYPE_SPAWN;
          for (int j = 0; j < 5; j++)
          {
            int prob = params[j];
            // >= min, < max
            event->spawn_point.probability_thresholds[j][0] = j == 0 ? 0 : event->spawn_point.probability_thresholds[j - 1][1];
            event->spawn_point.probability_thresholds[j][1] = event->spawn_point.probability_thresholds[j][0] + prob;
          }
          event->spawn_point.x = params[5];
          event->spawn_point.y = params[6];
        }

        if (!strcmp(s, "allow_firing"))
        {
          event->event_type = BFCONF_EVENT_TYPE_ALLOW_FIRING;
        }

        if (!strcmp(s, "disallow_firing"))
        {
          event->event_type = BFCONF_EVENT_TYPE_DISALLOW_FIRING;
        }
        if (!strcmp(s, "fire_in_circle"))
        {
          event->event_type = BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE;
          event->parameters[0] = params[0]; // number_of_directions
          event->parameters[1] = params[1]; // intensity
        }
        if (!strcmp(s, "modify_terrain"))
        {
          event->event_type = BFCONF_EVENT_TYPE_MODIFY_TERRAIN;
          event->parameters[0] = params[0]; // x
          event->parameters[1] = params[1]; // y
          if (!strcmp(s2, "floor"))
          {
            event->parameters[2] = BFCONF_MODIFY_TERRAIN_FLOOR;
          }
          if (!strcmp(s2, "wall"))
          {
            event->parameters[2] = BFCONF_MODIFY_TERRAIN_WALL;
          }
          if (!strcmp(s2, "level_exit"))
          {
            event->parameters[2] = BFCONF_MODIFY_TERRAIN_EXIT;
          }
        }
        if (!strcmp(s, "set_waypoint"))
        {
          event->event_type = BFCONF_EVENT_TYPE_SET_WAYPOINT;
          event->parameters[0] = params[0]; // x
          event->parameters[1] = params[1]; // y
          event->parameters[2] = params[2]; // waypoint_id
        }
        if (!strcmp(s, "clear_waypoint"))
        {
          event->event_type = BFCONF_EVENT_TYPE_CLEAR_WAYPOINT;
        }
        if (!strcmp(s, "start_secondary_timer"))
        {
          event->event_type = BFCONF_EVENT_TYPE_START_SECONDARY_TIMER;
          event->parameters[0] = params[0]; // time
        }
        if (!strcmp(s, "stop_secondary_timer"))
        {
          event->event_type = BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER;
        }
        if (!strcmp(s, "toggle_event_enabled"))
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

        if (!strcmp(s, "spawn_potion"))
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
  }

  config->state.previous_health = config->state.health + 1;
  print_configs(config);
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
    {
      int t_mask = 1 << econf->trigger_value;
      int t_en_mask = t_mask << 16;
      *trig = (state->positional_trigger_flags & t_mask) && !(state->positional_trigger_flags & t_en_mask);
    }
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

void bossfight_trigger_to_str(char *dst, int value)
{
  dst[0] = 0;
  switch (value)
  {
  case BFCONF_TRIGGER_TYPE_TIME_INTERVAL:
    strcpy(dst, "Time interval");
    break;
  case BFCONF_TRIGGER_TYPE_TIME_ONE_TIME:
    strcpy(dst, "Time one time");
    break;
  case BFCONF_TRIGGER_TYPE_HEALTH:
    strcpy(dst, "Health");
    break;
  case BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED:
    strcpy(dst, "Waypoint reached");
    break;
  case BFCONF_TRIGGER_TYPE_SECONDARY_TIMER:
    strcpy(dst, "Secondary timer");
    break;
  case BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED:
    strcpy(dst, "Kill count reached");
    break;
  case BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER:
    strcpy(dst, "positional trigger");
    break;
  case BFCONF_TRIGGER_TYPE_NEVER:
    strcpy(dst, "Never");
  default:
    dst[0] = 0;
    break;
  }
}

void bossfight_event_type_to_str(char *dst, int value)
{
  switch (value)
  {
  case BFCONF_EVENT_TYPE_NO_OP:
    strcpy(dst, "No-op");
    break;
  case BFCONF_EVENT_TYPE_SPAWN:
    strcpy(dst, "Spawn");
    break;
  case BFCONF_EVENT_TYPE_ALLOW_FIRING:
    strcpy(dst, "Allow firing");
    break;
  case BFCONF_EVENT_TYPE_DISALLOW_FIRING:
    strcpy(dst, "Disallow firing");
    break;
  case BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE:
    strcpy(dst, "Fire in circle");
    break;
  case BFCONF_EVENT_TYPE_MODIFY_TERRAIN:
    strcpy(dst, "Modify terrain");
    break;
  case BFCONF_EVENT_TYPE_SET_WAYPOINT:
    strcpy(dst, "Set waypoint");
    break;
  case BFCONF_EVENT_TYPE_CLEAR_WAYPOINT:
    strcpy(dst, "Clear waypoint");
    break;
  case BFCONF_EVENT_TYPE_START_SECONDARY_TIMER:
    strcpy(dst, "Start secondary timer");
    break;
  case BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER:
    strcpy(dst, "Stop secondary timer");
    break;
  case BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED:
    strcpy(dst, "Toggle event enabled");
    break;

  default:
    dst[0] = 0;
    break;
  }
}
