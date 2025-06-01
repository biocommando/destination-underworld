#pragma once

// The naming is a bit misleading: all scripted events use this mechanism; this just used
// to be only for boss fights.

#include <stdio.h>
#include "helpers.h"

#define BFCONF_MAX_EVENTS 100

// Never triggers. This is a placeholder used for overriding the event
// for a different game mode
#define BFCONF_TRIGGER_TYPE_NEVER 0
// Trigger always when the primary timer has counted this many ticks
#define BFCONF_TRIGGER_TYPE_TIME_INTERVAL 1
// Trigger only when primary timer has exactly this total tick count
#define BFCONF_TRIGGER_TYPE_TIME_ONE_TIME 2
// Trigger when boss enemy's health reaches this or below
#define BFCONF_TRIGGER_TYPE_HEALTH 3
// Trigger when boss enemy reaches its waypoint tile
#define BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED 4
// Trigger only when secondary timer has exactly this total tick count
#define BFCONF_TRIGGER_TYPE_SECONDARY_TIMER 5
// Trigger when player's kill count (in this room) has reached this or above
#define BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED 6
// Trigger when player enters a tile containing the corresponding positional trigger
#define BFCONF_TRIGGER_TYPE_POSITIONAL_TRIGGER 7

// Does nothing. This is a placeholder used for overriding the event
// for a different game mode
#define BFCONF_EVENT_TYPE_NO_OP 0
// Spawn an enemy using a probability mapper for choosing the enemy
// type that will be spawned
#define BFCONF_EVENT_TYPE_SPAWN 1
// Makes the boss enemy fire at will
#define BFCONF_EVENT_TYPE_ALLOW_FIRING 2
// Makes the boss enemy not fire at will
#define BFCONF_EVENT_TYPE_DISALLOW_FIRING 3
// Makes the boss enemy shoot a circle of fireballs
#define BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE 4
// Turn tiles to floor/wall/level exit
#define BFCONF_EVENT_TYPE_MODIFY_TERRAIN 5
// Set a waypoint for the boss enemy
#define BFCONF_EVENT_TYPE_SET_WAYPOINT 6
// Clear the current waypoint for the boss (return default behaviour)
#define BFCONF_EVENT_TYPE_CLEAR_WAYPOINT 7
// Start the secondary timer (initially stopped). The secondary timer differs
// from the primary timer in that:
// - it can be started and stopped
// - the start time can be set (primary timer is monotonous)
// - does not have interval triggering
#define BFCONF_EVENT_TYPE_START_SECONDARY_TIMER 8
// Stop the secondary timer
#define BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER 9
// Set a different event to enabled/disabled (won't trigger when disabled)
#define BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED 10
// Spawn a potion
#define BFCONF_EVENT_TYPE_SPAWN_POTION 11

// Terrain type for floor (for event type BFCONF_EVENT_TYPE_MODIFY_TERRAIN)
#define BFCONF_MODIFY_TERRAIN_FLOOR 1
// Terrain type for wall (for event type BFCONF_EVENT_TYPE_MODIFY_TERRAIN)
#define BFCONF_MODIFY_TERRAIN_WALL 2
// Terrain type for exit level (for event type BFCONF_EVENT_TYPE_MODIFY_TERRAIN)
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

/*
 * Read BossFightConfig structure from file.
 * Uses a syntax where each line consists of a command and any number of parameters
 * separated by spaces.
 *
 * The commands with parameters are (optional parameters marked with *):
 *
 * <property> amount override*
 *    Sets BossFightConfig.<property> to amount if game_modifiers == override or override
 *    parameter is not provided. The command <property> can be one of:
 *    health, speed, fire_rate, player_initial_gold
 *
 * time_starts_at time override*
 *    Set primary timer initially to this value (e.g. for fine tuning the first event from
 *    time interval triggers) if game_modifiers == override or override
 *    parameter is not provided.
 *
 * event
 *    New event. Event properties expected on next lines.
 *
 * event_override override
 *    Skips the next event property lines if game_modifiers != override.
 *
 * end
 *    End of event data.
 *
 * Event property commands:
 *
 * event_trigger type value
 *    Sets current event's trigger type and value (see BossFightEventConfig).
 *    Parameter type can be one of:
 *    time_interval, time_one_time, health, waypoint_reached, secondary_timer, kill_count,
 *    positional_trigger
 *
 * event_action spawn prob0 prob1 prob2 prob3 prob4 x y
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_SPAWN and configure spawn
 *    point so that the probability for enemy type 0 is prob0 %, enemy type 1 prob1 % and so on.
 *    prob0 + prob1 + ... + prob4 should equal 100. Set the map position x, y to the spawn point
 *    configuration.
 *
 * event_action allow_firing
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_ALLOW_FIRING.
 *
 * event_action disallow_firing
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_DISALLOW_FIRING.
 *
 * event_action fire_in_circle number_of_directions intensity
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE and parameters accordingly.
 *
 * event_action modify_terrain x y type
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_MODIFY_TERRAIN and parameters accordingly.
 *    The type can be one of: wall, floor, level_exit
 *
 * event_action set_waypoint x y waypoint_id
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_SET_WAYPOINT and parameters accordingly.
 *
 * event_action clear_waypoint
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_CLEAR_WAYPOINT.
 *
 * event_action start_secondary_timer time
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_START_SECONDARY_TIMER and the parameters accordingly.
 *
 * event_action stop_secondary_timer
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER.
 *
 * event_action toggle_event_enabled event_index enabled_status
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED and the parameters accordingly.
 *
 * event_action spawn_potion x y potion_type
 *    Sets current event's event_type to BFCONF_EVENT_TYPE_SPAWN_POTION and the parameters accordingly.
 *
 * event_initially_disabled
 *    Sets current event's enabled flag to 0.
 */
void read_bfconfig_new(FILE *f, BossFightConfig *config, int game_modifiers);

/*
 * Processes event triggers as described in BossFightEventConfig documentation.
 * Processing means that it checks each trigger condition and if the trigger condition
 * is met, marks the trigger value at index corresponding to event array index
 * as "triggered". The triggered events are then processed in the main game logic.
 */
void bossfight_process_event_triggers(BossFightConfig *config);

// BFCONF_TRIGGER_TYPE_* to string for debug prints
void bossfight_trigger_to_str(char *dst, int value);
// BFCONF_EVENT_TYPE_* to string for debug prints
void bossfight_event_type_to_str(char *dst, int value);

/*
 * Returns trigger state:
 *  0 = init
 *  1 = triggered
 * -1 = inactive
 */
int pos_trigger_state(const int *positional_trigger_flags, int ptrig_idx);

/*
 * Mark all positional triggers in triggered state to inactive state.
 */
void pos_trigger_clear(int *positional_trigger_flags);

/*
 * Set positional trigger state to triggered.
 */
void pos_trigger_set(int *positional_trigger_flags, int ptrig_idx);