#include "bossfightconf.h"

#include "iniRead.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

void print_configs(BossFightConfig *config)
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
  }
}

void read_bfconfig(FILE *f, BossFightConfig *config, int game_modifiers)
{
  memset(config, 0, sizeof(BossFightConfig));
  char main_section[100] = "main";
  char mode_override_key[100];
  sprintf(mode_override_key, "mode_override_%d", game_modifiers);
  ini_read_string_value(f, "main", mode_override_key, main_section);
  config->health = ini_read_int_value(f, main_section, "health");
  config->speed = ini_read_int_value(f, main_section, "speed");
  config->fire_rate = ini_read_int_value(f, main_section, "fire_rate");
  config->player_initial_gold = ini_read_int_value(f, main_section, "player_initial_gold");
  config->num_events = ini_read_int_value(f, "main", "events");
  if (config->num_events > BFCONF_MAX_EVENTS)
  {
    LOG("ERROR: too many events defined %d\n", config->num_events);
    config->num_events = BFCONF_MAX_EVENTS;
  }
  config->state.timer_value = ini_read_int_value(f, main_section, "time_starts_at");

  for (int i = 0; i < config->num_events; i++)
  {
    char event_segment[100];
    sprintf(event_segment, "event_%d", i);
    char s[256] = "";
    ini_read_string_value(f, event_segment, mode_override_key, s);
    if (s[0])
    {
      strncpy(event_segment, s, 99);
    }
    ini_read_string_value(f, event_segment, "trigger_type", s);

    config->events[i].enabled = !ini_read_int_value(f, event_segment, "initially_disabled");

    config->events[i].trigger_type = BFCONF_TRIGGER_TYPE_NEVER;
    config->events[i].event_type = BFCONF_EVENT_TYPE_NO_OP;

    if (!strcmp(s, "time_interval"))
    {
      config->events[i].trigger_type = BFCONF_TRIGGER_TYPE_TIME_INTERVAL;
    }
    if (!strcmp(s, "time_one_time"))
    {
      config->events[i].trigger_type = BFCONF_TRIGGER_TYPE_TIME_ONE_TIME;
    }
    if (!strcmp(s, "health"))
    {
      config->events[i].trigger_type = BFCONF_TRIGGER_TYPE_HEALTH;
    }
    if (!strcmp(s, "waypoint_reached"))
    {
      config->events[i].trigger_type = BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED;
    }
    if (!strcmp(s, "secondary_timer"))
    {
      config->events[i].trigger_type = BFCONF_TRIGGER_TYPE_SECONDARY_TIMER;
    }
    if (!strcmp(s, "kill_count"))
    {
      config->events[i].trigger_type = BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED;
    }
    config->events[i].trigger_value = ini_read_int_value(f, event_segment, "trigger_value");

    ini_read_string_value(f, event_segment, "event_type", s);

    if (!strcmp(s, "spawn"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_SPAWN;
      int spawn_point = ini_read_int_value(f, event_segment, "spawn_point");
      char spawn_segment[100];
      sprintf(spawn_segment, "spawn_point_%d", spawn_point);
      for (int j = 0; j < 5; j++)
      {
        char key[100];
        sprintf(key, "enemy_%d_probability", j);
        int prob = ini_read_int_value(f, spawn_segment, key);
        // >= min, < max
        config->events[i].spawn_point.probability_thresholds[j][0] = j == 0 ? 0 : config->events[i].spawn_point.probability_thresholds[j - 1][1];
        config->events[i].spawn_point.probability_thresholds[j][1] = config->events[i].spawn_point.probability_thresholds[j][0] + prob;
      }
      config->events[i].spawn_point.x = ini_read_int_value(f, spawn_segment, "x");
      config->events[i].spawn_point.y = ini_read_int_value(f, spawn_segment, "y");
    }

    if (!strcmp(s, "allow_firing"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_ALLOW_FIRING;
    }

    if (!strcmp(s, "disallow_firing"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_DISALLOW_FIRING;
    }
    if (!strcmp(s, "fire_in_circle"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE;
      config->events[i].parameters[0] = ini_read_int_value(f, event_segment, "number_of_directions");
      config->events[i].parameters[1] = ini_read_int_value(f, event_segment, "intensity");
    }
    if (!strcmp(s, "modify_terrain"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_MODIFY_TERRAIN;
      config->events[i].parameters[0] = ini_read_int_value(f, event_segment, "x");
      config->events[i].parameters[1] = ini_read_int_value(f, event_segment, "y");
      char terrain_type[100];
      ini_read_string_value(f, event_segment, "terrain_type", terrain_type);
      if (!strcmp(terrain_type, "floor"))
      {
        config->events[i].parameters[2] = BFCONF_MODIFY_TERRAIN_FLOOR;
      }
      if (!strcmp(terrain_type, "wall"))
      {
        config->events[i].parameters[2] = BFCONF_MODIFY_TERRAIN_WALL;
      }
      if (!strcmp(terrain_type, "level_exit"))
      {
        config->events[i].parameters[2] = BFCONF_MODIFY_TERRAIN_EXIT;
      }
    }
    if (!strcmp(s, "set_waypoint"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_SET_WAYPOINT;
      config->events[i].parameters[0] = ini_read_int_value(f, event_segment, "x");
      config->events[i].parameters[1] = ini_read_int_value(f, event_segment, "y");
      config->events[i].parameters[2] = ini_read_int_value(f, event_segment, "waypoint_id");
    }
    if (!strcmp(s, "clear_waypoint"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_CLEAR_WAYPOINT;
    }
    if (!strcmp(s, "start_secondary_timer"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_START_SECONDARY_TIMER;
      config->events[i].parameters[0] = ini_read_int_value(f, event_segment, "time");
    }
    if (!strcmp(s, "stop_secondary_timer"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER;
    }
    if (!strcmp(s, "toggle_event_enabled"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED;
      int event_id = ini_read_int_value(f, event_segment, "event_id");
      if (event_id < 0 || event_id >= config->num_events)
      {
        LOG("Invalid event_id field %d\n", event_id);
        event_id = 0;
      }
      config->events[i].parameters[0] = event_id;
      config->events[i].parameters[1] = ini_read_int_value(f, event_segment, "enabled");
    }

    if (!strcmp(s, "spawn_potion"))
    {
      config->events[i].event_type = BFCONF_EVENT_TYPE_SPAWN_POTION;
      config->events[i].parameters[0] = ini_read_int_value(f, event_segment, "x");
      config->events[i].parameters[1] = ini_read_int_value(f, event_segment, "y");
      config->events[i].parameters[2] = ini_read_int_value(f, event_segment, "type");
    }
  }

  print_configs(config);
}

void bossfight_process_event_triggers(BossFightConfig *config)
{
  BossFightState *state = &config->state;
  if (state->timer_value == 0)
  {
    state->previous_health = state->health + 1;
  }
  if (state->secondary_timer_started)
  {
    state->secondary_timer_value++;
  }
  state->timer_value++;
  int tv = state->timer_value;
  for (int i = 0; i < BFCONF_MAX_EVENTS; i++)
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
