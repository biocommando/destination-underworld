#ifndef DU_BOSS_FIGHT_CONFIG
#define DU_BOSS_FIGHT_CONFIG

#include <stdio.h>

#define BFCONF_MAX_EVENTS 100

#define BFCONF_TRIGGER_TYPE_TIME_INTERVAL 'i'
#define BFCONF_TRIGGER_TYPE_TIME_ONE_TIME 'o'
#define BFCONF_TRIGGER_TYPE_HEALTH 'h'
#define BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED 'w'
#define BFCONF_TRIGGER_TYPE_SECONDARY_TIMER 's'
#define BFCONF_TRIGGER_TYPE_NEVER 'N'

#define BFCONF_EVENT_TYPE_NO_OP '0'
#define BFCONF_EVENT_TYPE_SPAWN 's'
#define BFCONF_EVENT_TYPE_ALLOW_FIRING 'a'
#define BFCONF_EVENT_TYPE_DISALLOW_FIRING 'd'
#define BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE 'c'
#define BFCONF_EVENT_TYPE_MODIFY_TERRAIN 't'
#define BFCONF_EVENT_TYPE_SET_WAYPOINT 'w'
#define BFCONF_EVENT_TYPE_CLEAR_WAYPOINT 'r'
#define BFCONF_EVENT_TYPE_START_SECONDARY_TIMER '2'
#define BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER 'S'
#define BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED 'T'

#define BFCONF_MODIFY_TERRAIN_FLOOR 1
#define BFCONF_MODIFY_TERRAIN_WALL 2
#define BFCONF_MODIFY_TERRAIN_EXIT 3

typedef struct
{
  int x;
  int y;
  int probability_thresholds[5][2];
} BossFightSpawnPointConfig;

typedef struct
{
  int trigger_type;
  int trigger_value;
  int event_type;
  int enabled;
  BossFightSpawnPointConfig spawn_point;
  int parameters[3];
} BossFightEventConfig;

typedef struct
{
  int health;
  int previous_health;
  int timer_value;
  int secondary_timer_value;
  int secondary_timer_started;
  int waypoint;
  int waypoint_reached;
  int sees_player;
  int triggers[BFCONF_MAX_EVENTS];
} BossFightState;

typedef struct
{
  int health;
  int fire_rate;
  int speed;
  int player_initial_gold;
  int num_events;
  BossFightEventConfig events[BFCONF_MAX_EVENTS];
  BossFightState state;
} BossFightConfig;

void read_bfconfig(FILE *f, BossFightConfig *config, int game_modifiers);

void bossfight_process_event_triggers(BossFightConfig *config);

#endif
