#include "enemy_logic.h"
#include "logging.h"
#include "predictableRandom.h"
#include "worldInteraction.h"
#include "renderWorld.h"
#include "sprites.h"
#include "sampleRegister.h"
#include "vfx.h"
#include <math.h>

static inline void set_directions(Enemy *enm, Coordinates *aim_at, int aim_window)
{
  if (enm->x > aim_at->x + aim_window)
    enm->dx = -1;
  if (enm->x < aim_at->x - aim_window)
    enm->dx = 1;
  if (enm->y > aim_at->y + aim_window)
    enm->dy = -1;
  if (enm->y < aim_at->y - aim_window)
    enm->dy = 1;
}

void enemy_logic(World *world)
{
  for (int x = 0; x < ENEMYCOUNT; x++)
  {
    Enemy *enm = &world->enm[x];
    if (!enm->alive)
    {
      const int death_anim_max = 16;
      if (enm->death_animation < death_anim_max)
      {
        if (enm->roomid == world->current_room)
        {
          draw_sprite_animated_centered(world->spr, SPRITE_ID_SKELETON, enm->x, enm->y, 0, enm->death_animation * 4 / death_anim_max);
          enm->death_animation++;
          if (enm->death_animation == death_anim_max)
          {
            spawn_body_parts(enm, &world->visual_fx);
            trigger_sample_with_params(SAMPLE_SPLASH(rand() % 3), 180 + rand() % 76, 127, 500 + rand() % 500);
          }
        }
        else
        {
          enm->death_animation = death_anim_max;
          spawn_body_parts(enm, &world->visual_fx);
        }
      }
      continue;
    }
    if (enm->roomid != world->current_room)
      continue;

    if (world->plr.health > 0)
    {
      int is_boss = enm == world->boss;
      if (enm->turret != TURRET_TYPE_PLAYER) // not a (player's) turret
      {
        Coordinates aim_at = {world->plr.x, world->plr.y};
        int aim_window = 2 + (enm->turret || is_boss ? 5 : 0);
        int reacts_to_player = sees_each_other(enm, &world->plr, world);

        if (reacts_to_player || (is_boss && world->boss_fight_config->state.boss_waypoint.x >= 0))
        {
          enm->move = 1;
          if (!is_boss || (world->boss_fight_config->state.boss_want_to_shoot && reacts_to_player))
          {
            if (is_boss)
            {
              set_directions(enm, &aim_at, aim_window);
            }
            if (!check_potion_effect(world, POTION_EFFECT_STOP_ENEMIES))
            {
              int play_sample = shoot(enm, world);
              if (play_sample)
              {
                trigger_sample(SAMPLE_THROW, 255);
              }
            }
          }

          enm->dx = enm->dy = 0;
          if (is_boss && world->boss_fight_config->state.boss_waypoint.x >= 0)
          {
            aim_at.x = TO_PIXEL_COORDINATES(world->boss_fight_config->state.boss_waypoint.x);
            aim_at.y = TO_PIXEL_COORDINATES(world->boss_fight_config->state.boss_waypoint.y);
            aim_window = 0;
            if (enm->x / TILESIZE == (int)world->boss_fight_config->state.boss_waypoint.x && enm->y / TILESIZE == (int)world->boss_fight_config->state.boss_waypoint.y)
            {
              LOG_TRACE("Waypoint reached\n");
              world->boss_fight_config->state.boss_waypoint.x = world->boss_fight_config->state.boss_waypoint.y = -1;
              world->boss_fight_config->state.waypoint_reached = 1;
            }
          }
          set_directions(enm, &aim_at, aim_window);
          if (enm->dx == 0 && enm->dy == 0)
          {
            enm->dx = 1 - 2 * pr_get_random() % 2;
            enm->dy = 1 - 2 * pr_get_random() % 2;
          }
        }
        else
        {
          if (pr_get_random() % 30 == 0)
          {
            enm->move = pr_get_random() % 2;
            enm->move = pr_get_random() % 2;
            enm->dx = 1 - (pr_get_random() % 3);
            enm->dy = 1 - (pr_get_random() % 3);
          }
        }
        if (!check_potion_effect(world, POTION_EFFECT_STOP_ENEMIES))
        {
          int speed = 1;
          if (is_boss)
            speed = world->boss_fight_config->speed;
          else if (enm->turret == TURRET_TYPE_ENEMY)
            speed = 0;
          else if (enm->fast)
            speed = 2;

          enemy_reload(enm, speed ? speed : 1);

          for (int m = 0; m < speed; m++)
            move_enemy(enm, world);
        }
        draw_enemy(enm, world);
      }
      else // turret
      {
        enemy_reload(enm, enm->move);
        if (enm->move > 0)
        {
          for (int i = 0; i < enm->move; i++)
            move_enemy(enm, world);
          enm->move--;
        }
        else
        {
          float circular = cos((float)(enm->ammo / 4 % 32) * M_PI / 16) * 4;
          enm->dx = (int)circular;
          circular = sin((float)(enm->ammo / 4 % 32) * M_PI / 16) * 4;
          enm->dy = (int)circular;

          int play_sample = shoot(enm, world);
          if (play_sample)
          {
            trigger_sample(SAMPLE_THROW, 255);
          }
        }
        draw_enemy(enm, world);
        if (enm->ammo == 0)
        {
          trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
          create_shade_around_hit_point(enm->x, enm->y, enm->roomid, 9, &world->visual_fx);
          create_explosion(enm->x, enm->y, world, &world->visual_fx, 1);
          create_explosion(enm->x, enm->y, world, &world->visual_fx, 1);
          create_explosion(enm->x, enm->y, world, &world->visual_fx, 2);
          if (world->plr.perks & PERK_IMPROVE_TURRET_POWERUP)
            create_cluster_explosion(world, enm->x, enm->y, 32, 1, &world->plr);
          enm->ammo = -1;
          enm->shots = 1;
          enm->alive = 0;
        }
      }
    }
    else
      draw_enemy(enm, world);
  }
}