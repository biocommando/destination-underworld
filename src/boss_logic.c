#include "boss_logic.h"
#include "logging.h"
#include "predictableRandom.h"
#include "worldInteraction.h"
#include "sampleRegister.h"
#include "bossfightconf.h"
#include "vfx.h"
#include <math.h>

void boss_logic(World *world, int boss_died)
{
  Enemy *boss = world->boss;
  int in_same_room = boss != NULL && boss->roomid == world->current_room;
  if (boss)
    world->boss_fight_config->state.health = boss_died ? 0 : boss->health;
  // world->boss_fight_config->state.player_kills = world->kills;
  bossfight_process_event_triggers(world->boss_fight_config);
  // Ensure that positional triggers will only fire once ever
  world->boss_fight_config->state.positional_trigger_flags |=
      (world->boss_fight_config->state.positional_trigger_flags & 0xFFFF) << 16;
  for (int x = 0; x < world->boss_fight_config->num_events; x++)
  {
    if (!world->boss_fight_config->state.triggers[x])
      continue;

    BossFightEventConfig *event = &world->boss_fight_config->events[x];

    char s[100];
    bossfight_event_type_to_str(s, event->event_type);
    LOG_TRACE("Trigger %s\n", s);
    if (!event->enabled)
    {
      LOG_TRACE("Event disabled\n");
      continue;
    }
    switch (event->event_type)
    {
    case BFCONF_EVENT_TYPE_SPAWN:
    {
      BossFightSpawnPointConfig *spawn_point = &event->spawn_point;
      int random_num = pr_get_random() % 100;
      for (int spawn_type = 0; spawn_type < 5; spawn_type++)
      {
        if (random_num >= spawn_point->probability_thresholds[spawn_type][0] && random_num < spawn_point->probability_thresholds[spawn_type][1])
        {
          spawn_enemy(spawn_point->x, spawn_point->y, spawn_type, world->current_room, world);
          create_sparkles(TO_PIXEL_COORDINATES(spawn_point->x), TO_PIXEL_COORDINATES(spawn_point->y), 15, 2, 20, world);

          trigger_sample(SAMPLE_SPAWN, 255);
          break;
        }
      }
    }
    break;
    case BFCONF_EVENT_TYPE_ALLOW_FIRING:
      world->boss_fight_config->state.boss_want_to_shoot = 1;
      break;
    case BFCONF_EVENT_TYPE_DISALLOW_FIRING:
      world->boss_fight_config->state.boss_want_to_shoot = 0;
      break;
    case BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE:
      if (in_same_room)
        create_cluster_explosion(world, boss->x, boss->y, event->parameters[0], event->parameters[1], boss);
      break;
    case BFCONF_EVENT_TYPE_MODIFY_TERRAIN:
    {
      int tile_type = 0;
      switch (event->parameters[2])
      {
      case BFCONF_MODIFY_TERRAIN_FLOOR:
        tile_type = TILE_SYM_FLOOR;
        break;
      case BFCONF_MODIFY_TERRAIN_WALL:
        tile_type = TILE_SYM_WALL1;
        break;
      case BFCONF_MODIFY_TERRAIN_EXIT:
        tile_type = TILE_SYM_EXIT_LEVEL;
        break;
      }
      if (tile_type)
      {
        world->map[world->current_room - 1][event->parameters[0]][event->parameters[1]] = create_tile(tile_type);
        trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
        for (int y = 0; y < 3; y++)
          create_explosion(event->parameters[0] * TILESIZE + TILESIZE / 2, event->parameters[1] * TILESIZE + TILESIZE / 2, world, 1);
      }
    }
    break;
    case BFCONF_EVENT_TYPE_SET_WAYPOINT:
      world->boss_fight_config->state.boss_waypoint.x = event->parameters[0];
      world->boss_fight_config->state.boss_waypoint.y = event->parameters[1];
      world->boss_fight_config->state.waypoint = event->parameters[2];
      world->boss_fight_config->state.waypoint_reached = 0;
      break;
    case BFCONF_EVENT_TYPE_CLEAR_WAYPOINT:
      world->boss_fight_config->state.boss_waypoint.x = -1;
      world->boss_fight_config->state.boss_waypoint.y = -1;
      world->boss_fight_config->state.waypoint = 0;
      break;
    case BFCONF_EVENT_TYPE_START_SECONDARY_TIMER:
      world->boss_fight_config->state.secondary_timer_started = 1;
      if (event->parameters[0] >= 0)
        world->boss_fight_config->state.secondary_timer_value = event->parameters[0];
      break;
    case BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER:
      world->boss_fight_config->state.secondary_timer_started = 0;
      break;
    case BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED:
      world->boss_fight_config->events[event->parameters[0]].enabled = event->parameters[1];
      break;
    case BFCONF_EVENT_TYPE_SPAWN_POTION:
      spawn_potion(TO_PIXEL_COORDINATES(event->parameters[0]), TO_PIXEL_COORDINATES(event->parameters[1]),
                   event->parameters[2], world->current_room, world, POTION_PRESET_RANGE_START, POTION_PRESET_RANGE_END);
      create_sparkles(TO_PIXEL_COORDINATES(event->parameters[0]), TO_PIXEL_COORDINATES(event->parameters[1]), 15, 2, 15, world);
      break;
    }
  }
}