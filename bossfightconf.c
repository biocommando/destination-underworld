#include "bossfightconf.h"

#include "iniRead.h"
#include <stdlib.h>
#include <string.h>


void print_configs(BossFightConfig *config)
{ 
  printf("Bossfight config read:\n  Events: %d\n", config->num_events);
  for (int i = 0; i < config->num_events && i < BFCONF_MAX_EVENTS; i++)
  {
    printf("  Event %d\n    Event type: %c\n", i, config->events[i].event_type);
    printf("    Params: %d %d %d\n", config->events[i].parameters[0], config->events[i].parameters[1], config->events[i].parameters[2]);
    printf("    Trigger type / value: %c / %d\n", config->events[i].trigger_type, config->events[i].trigger_value);
    if(config->events[i].event_type == BFCONF_EVENT_TYPE_SPAWN)
    {
      printf("    Spawn details: x %d y %d\n    ", config->events[i].spawn_point.x, config->events[i].spawn_point.y);
      for (int j = 0; j < 5; j++)
      {
        printf(" %d: %d - %d %%", j, config->events[i].spawn_point.probability_thresholds[j][0],
          config->events[i].spawn_point.probability_thresholds[j][1]);
      }
      printf("\n");
    }
  }
}

void read_bfconfig(FILE *f, BossFightConfig *config)
{
 config->health = ini_read_int_value(f, "main", "health");
 config->speed = ini_read_int_value(f, "main", "speed");
 config->fire_rate = ini_read_int_value(f, "main", "fire_rate");
 config->player_initial_gold = ini_read_int_value(f, "main", "player_initial_gold");
 config->num_events = ini_read_int_value(f, "main", "events");
 config->state.timer_value = ini_read_int_value(f, "main", "time_starts_at");
 
 for (int i = 0; i < config->num_events && i < BFCONF_MAX_EVENTS; i++)
 {
  char event_segment[100];
  sprintf(event_segment, "event_%d", i);
  char s[256];
  ini_read_string_value(f, event_segment, "trigger_type", s);
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
 }

  print_configs(config);
}


void bossfight_process_event_triggers(BossFightConfig *config)
{
 BossFightState* state = &config->state;
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
  switch(econf->trigger_type)
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
    default:
         *trig = 0;
         break;
  }
 }
 state->previous_health = state->health;
 state->waypoint_reached = 0;
}
