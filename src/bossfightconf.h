#ifndef DU_BOSS_FIGHT_CONFIG
#define DU_BOSS_FIGHT_CONFIG

#include <stdio.h>

#define BFCONF_MAX_EVENTS 100

#define BFCONF_TRIGGER_TYPE_NEVER 0
#define BFCONF_TRIGGER_TYPE_TIME_INTERVAL 1
#define BFCONF_TRIGGER_TYPE_TIME_ONE_TIME 2
#define BFCONF_TRIGGER_TYPE_HEALTH 3
#define BFCONF_TRIGGER_TYPE_WAYPOINT_REACHED 4
#define BFCONF_TRIGGER_TYPE_SECONDARY_TIMER 5
#define BFCONF_TRIGGER_TYPE_PLAYER_KILLCOUNT_REACHED 6

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
  int player_kills;
  int player_previous_kills;
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

void bossfight_trigger_to_str(char *dst, int value);
void bossfight_event_type_to_str(char *dst, int value);

#endif
