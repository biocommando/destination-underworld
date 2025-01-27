#pragma once

// The naming is a bit misleading: all scripted events use this mechanism; this just used
// to be only for boss fights.

#include <stdio.h>
#include "helpers.h"

#define BFCONF_MAX_EVENTS 100

#define BFCONF_TRIGGER_TYPE_NEVER 0
#define BFCONF_TRIGGER_TYPE_TIME_INTERVAL 1
#define BFCONF_TRIGGER_TYPE_TIME_ONE_TIME 2
#define BFCONF_TRIGGER_TYPE_HEALTH 3
#define BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED 4
#define BFCONF_TRIGGER_TYPE_SECONDARY_TIMER 5
#define BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED 6
#define BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER 7

#define BFCONF_EVENT_TYPE_NO_OP 0
#define BFCONF_EVENT_TYPE_SPAWN 1
#define BFCONF_EVENT_TYPE_ALLOW_FIRING 2
#define BFCONF_EVENT_TYPE_DISALLOW_FIRING 3
#define BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE 4
#define BFCONF_EVENT_TYPE_MODIFY_TERRAIN 5
#define BFCONF_EVENT_TYPE_SET_WAYPOINT 6
#define BFCONF_EVENT_TYPE_CLEAR_WAYPOINT 7
#define BFCONF_EVENT_TYPE_START_SECONDARY_TIMER 8
#define BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER 9
#define BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED 10
#define BFCONF_EVENT_TYPE_SPAWN_POTION 11

#define BFCONF_MODIFY_TERRAIN_FLOOR 1
#define BFCONF_MODIFY_TERRAIN_WALL 2
#define BFCONF_MODIFY_TERRAIN_EXIT 3

/*
 * Spawn point for enemies. Means that enemies spawn to these coordinates
 * with certain probabilities.
 */
typedef struct
{
  // x, y coordinates (in map coordinates)
  int x, y;
  // Min/max probability for each enemy type to spawn
  int probability_thresholds[5][2];
} BossFightSpawnPointConfig;

// Bossfight scripting event
typedef struct
{
  // The type of trigger that fires this event.
  // Defined by the BFCONF_TRIGGER_TYPE_* defines.
  int trigger_type;
  /*
   * Trigger value associated with the trigger type.
   * Meaning for different trigger types:
   * - never: N/A
   * - time interval: interval in "bossfight ticks"
   * - time one time: trigger time in "bossfight ticks"
   * - health: boss health
   * - waypoint reached: waypoint id
   * - secondary timer: trigger time in "bossfight ticks"
   * - killcount reached: player kills (in this room)
   * - positional trigger: positional trigger index
   */
  int trigger_value;
  // The event type, defined by BFCONF_EVENT_TYPE_*
  int event_type;
  // Is the event enabled? (Can be toggled by other events)
  int enabled;
  // Spawn point configuration used by BFCONF_EVENT_TYPE_SPAWN events
  BossFightSpawnPointConfig spawn_point;
  /*
   * Event parameters, different meaning for each event type:
   * - no-op: N/A
   * - spawn: N/A (parameters determined by the spawn_point field)
   * - allow firing: N/A
   * - disallow firing: N/A
   * - fire in circle: 0 -> number of directions,
   *                   1 -> intensity
   *                   for create_cluster_explosion function
   * - modify terrain: 0 -> x coordinate
   *                   1 -> y coordinate
   *                   2 -> terrain type to spawn (BFCONF_MODIFY_TERRAIN_*)
   * - set waypoint: 0 -> waypoint x coordinate
   *                 1 -> waypoint y coordinate
   *                 2 -> waypoint id
   * - clear waypoint: N/A
   * - start secondary timer: 0 -> timer start time (if >= 0)
   * - toggle event enabled: 0 -> event index
   *                         1 -> enabled state
   * - spawn potion: 0 -> potion x coordinate
   *                 1 -> potion y coordinate
   *                 2 -> potion type id
   */
  int parameters[3];
} BossFightEventConfig;

/*
 * Scripting state for a single room in a level.
 */
typedef struct
{
  // Boss current health
  int health;
  // Boss health after the last time event triggers were processed
  int previous_health;
  // The primary timer value ("bossfight ticks")
  int timer_value;
  // The secondary timer value ("bossfight ticks"). Incremented only if secondary_timer_started is set to 1.
  int secondary_timer_value;
  // Set to 1 to make secondary_timer_value to increment
  int secondary_timer_started;
  // Waypoint id. This is set when the waypoint is set and reaching a certain waypoint
  // can trigger other events.
  int waypoint;
  // Has the waypoint been reached after the last "bossfight tick"?
  int waypoint_reached;
  // True if boss sees player
  int sees_player;
  // Number of kills in this room
  int player_kills;
  // Number of kills in this room after the last time event triggers were processed
  int player_previous_kills;
  // 1/0 flag for each BossFightConfig.events index if the event should be triggered
  // after the last time event triggers were processed. Triggers are reset to 0
  // during trigger processing if the trigger conditions are not met.
  int triggers[BFCONF_MAX_EVENTS];
  // x, y coordinates where the boss tries to walk. -1, -1 = waypoint not set
  Coordinates boss_waypoint;
  // True if boss can fire at will
  int boss_want_to_shoot;
  // Tracks the current positional trigger flags. Least significant 16 bits hold
  // the state if player has entered the positional trigger tile. Most significant
  // 16 bits hold the state if the positional trigger has already been processed.
  // So e.g. positional trigger number 0 will only fire if bit at 0 is 1 and bit at 16
  // is 0.
  int positional_trigger_flags;
} BossFightState;

/*
 * Scripting configuration for a single room in a level.
 */
typedef struct
{
  // Boss initial health (if exists)
  int health;
  // Boss fire rate (if exists)
  int fire_rate;
  // Boss speed (if exists)
  int speed;
  // Player initial gold override. -1 = no override
  int player_initial_gold;
  // Number of events in events array
  int num_events;
  BossFightEventConfig events[BFCONF_MAX_EVENTS];
  // Current state
  BossFightState state;
} BossFightConfig;

void read_bfconfig_new(FILE *f, BossFightConfig *config, int game_modifiers);

void bossfight_process_event_triggers(BossFightConfig *config);

// BFCONF_TRIGGER_TYPE_* to string for debug prints
void bossfight_trigger_to_str(char *dst, int value);
// BFCONF_EVENT_TYPE_* to string for debug prints
void bossfight_event_type_to_str(char *dst, int value);