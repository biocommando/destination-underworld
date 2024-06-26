#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "logging.h"
#include "dump3.h"
#include "duconstants.h"
#include "ducolors.h"
#include "world.h"
#include "worldInteraction.h"
#include "renderWorld.h"
#include "gamePersistence.h"
#include "menu.h"
#include "bossfightconf.h"
#include "predictableRandom.h"
#include "settings.h"
#include "continuousData.h"
#include "keyhandling.h"
#include "sampleRegister.h"
#include "helpers.h"
#include "loadindicator.h"
#include "sprites.h"

#ifdef ENABLE_LOGGING
int logging_enabled = 0;
#endif

int game(int mission, int *game_modifiers);

GameSettings game_settings;

Enemy plrautosave;

int fname_counter = 0;

int record_mode = RECORD_MODE_NONE;
FILE *record_input_file = NULL;
char record_input_filename[256];
int no_player_damage = 0;

int main(int argc, char **argv)
{
  char read_arg[256] = "";
#ifdef ENABLE_LOGGING
  logging_enabled = read_cmd_line_arg_int("logging", argv, argc);
#endif
  read_cmd_line_arg_str("player-damage", argv, argc, read_arg);
  if (!strcmp(read_arg, "off"))
  {
    no_player_damage = 1;
  }
  read_arg[0] = 0;

  read_cmd_line_arg_str("record-mode", argv, argc, read_arg);
  if (!strcmp(read_arg, "record"))
  {
    record_mode = RECORD_MODE_RECORD;
    LOG("Record mode active.\n");
  }
  else if (!strcmp(read_arg, "play"))
  {
    record_mode = RECORD_MODE_PLAYBACK;
    char fname[256];
    if (read_cmd_line_arg_str("file", argv, argc, fname))
    {
      record_input_file = fopen(fname, "r");
      strcpy(record_input_filename, fname);
    }
    if (record_input_file == NULL)
    {
      LOG("Valid input file required (--file=<filename>)!!\n");
      return 0;
    }
    LOG("Playback mode active.\n");
  }

  read_settings(argv, argc);
  init_allegro();
  progress_load_state("Loading game...", 1);
  preload_mp3s();
  srand((int)time(NULL));
  int mission = 1;
  int game_modifiers = 0;

  progress_load_state("Loading samples...", 1);
  register_sample(SAMPLE_SELECT, "sel.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_WARP, "warp.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_BOSSTALK_1, "bt1.wav", SAMPLE_PRIORITY(HIGH, 2));
  register_sample(SAMPLE_BOSSTALK_2, "bt2.wav", SAMPLE_PRIORITY(HIGH, 2));
  register_sample(SAMPLE_THROW, "throw.wav", SAMPLE_PRIORITY(LOW, 0));
  register_sample(SAMPLE_SELECT_WEAPON, "select_weapon.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_HEAL, "healing.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_PROTECTION, "rune_of_protection.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_TURRET, "turret.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_SPAWN, "spawn.wav", SAMPLE_PRIORITY(HIGH, 0));
  register_sample(SAMPLE_POTION(0), "potion_shield.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_POTION(1), "potion_stop.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_POTION(2), "potion_fast.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_POTION(3), "potion_boost.wav", SAMPLE_PRIORITY(HIGH, 1));
  register_sample(SAMPLE_POTION(4), "potion_heal.wav", SAMPLE_PRIORITY(HIGH, 1));

  for (int i = 0; i < 6; i++)
  {
    char loadsamplename[100];
    sprintf(loadsamplename, "ex%d.wav", i + 1);
    register_sample(SAMPLE_EXPLOSION(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0));
    sprintf(loadsamplename, "die%d.wav", i + 1);
    register_sample(SAMPLE_DEATH(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0));
  }

  progress_load_state("Loading sprites...", 1);
  {
    char path[256];
    get_data_filename(path, "sprites.dat");
    read_sprites_from_file(path, SPRITE_ID_MIN, SPRITE_ID_MAX);
  }

  switch_track(1);
  progress_load_state("Loading menu...", 1);

  while (mission != 0)
  {
    menu(0, &plrautosave, &mission, &game_modifiers);
    while (mission > 0)
    {
      mission = game(mission, &game_modifiers);
      if (record_input_file)
        break;
    }
  }
  progress_load_state("Exiting game...", 0);
  destroy_registered_samples();
  if (record_input_file)
    fclose(record_input_file);

  destroy_mp3s();
  /*set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  remove_keyboard();
  remove_mouse();*/
  return 0;
}

void set_directions(Enemy *enm, Coordinates *aim_at, int aim_window)
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
    if (enm->roomid != world->current_room || enm->id == NO_OWNER)
      continue;

    if (world->plr.health > 0)
    {
      int is_boss = enm == world->boss;
      if (enm->turret != TURRET_TYPE_PLAYER) // not a (player's) turret
      {
        Coordinates aim_at = {world->plr.x, world->plr.y};
        int aim_window = 2 + (enm->turret || enm == world->boss ? 5 : 0);
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
            aim_at.x = world->boss_fight_config->state.boss_waypoint.x * TILESIZE + HALFTILESIZE;
            aim_at.y = world->boss_fight_config->state.boss_waypoint.y * TILESIZE + HALFTILESIZE;
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
        if (is_boss)
        {
          for (int m = 0; m < world->boss_fight_config->speed; m++)
            move_enemy(enm, world);
        }
        else if (enm->turret == TURRET_TYPE_NONE)
        {
          if (!check_potion_effect(world, POTION_EFFECT_STOP_ENEMIES))
          {
            if (enm->fast)
            {
              move_enemy(enm, world);
            }
            move_enemy(enm, world);
          }
        }
        else if (enm->reload > 0)
        {
          enm->reload--;
        }
        draw_enemy(enm, world);
      }
      else // turret
      {
        if (enm->move > 0)
        {
          for (int i = 0; i < enm->move; i++)
            move_enemy(enm, world);
          enm->move--;
        }
        else
        {
          float circular = cos((float)(enm->ammo / 4 % 32) * M_PI / 16) * 4;
          enm->dx = (int)circular; //(circular > 0.1) - 1 * (circular < -0.1);
          circular = sin((float)(enm->ammo / 4 % 32) * M_PI / 16) * 4;
          enm->dy = (int)circular; //(circular > 0.1) - 1 * (circular < -0.1);

          int play_sample = shoot(enm, world);
          if (play_sample)
          {
            trigger_sample(SAMPLE_THROW, 255);
          }

          enm->reload--;
        }
        draw_enemy(enm, world);
        if (enm->ammo == 0)
        {
          trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
          create_shade_around_hit_point(enm->x, enm->y, 9, world);
          create_explosion(enm->x, enm->y, world);
          create_explosion(enm->x, enm->y, world);
          create_explosion(enm->x, enm->y, world);
          enm->ammo = -1;
          enm->shots = 1;
          enm->id = NO_OWNER;
        }
      }
    }
    else
      draw_enemy(enm, world);
  }
}

void draw_static_background()
{
  rectfill(0, 0, SCREEN_W, SCREEN_H, BLACK);
  int maxsz = SCREEN_H > SCREEN_W ? SCREEN_H : SCREEN_W;
  for (int i = 0; i < maxsz / 2; i += maxsz / 80)
  {
    al_draw_circle(SCREEN_W / 2,
                   SCREEN_H / 2,
                   i,
                   al_map_rgb(33, 33, 33), 1);
  }
}

void boss_logic(World *world, int boss_died)
{
  Enemy *boss = world->boss;
  int in_same_room = boss != NULL && boss->roomid == world->current_room;
  if (/*in_same_room || boss_died*/1)
  {
    if (boss)
      world->boss_fight_config->state.health = boss_died ? 0 : boss->health;
    //world->boss_fight_config->state.player_kills = world->kills;
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
            create_sparkles(spawn_point->x * TILESIZE + HALFTILESIZE, spawn_point->y * TILESIZE + HALFTILESIZE, 15, 2, world);

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
            create_explosion(event->parameters[0] * TILESIZE + TILESIZE / 2, event->parameters[1] * TILESIZE + TILESIZE / 2, world);
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
        spawn_potion(event->parameters[0] * TILESIZE + HALFTILESIZE, event->parameters[1] * TILESIZE + HALFTILESIZE,
                     event->parameters[2], world->current_room, world);
        break;
      }
    }
  }
}

void bullet_logic(World *world)
{
  const int difficulty = GET_DIFFICULTY(world);
  for (int i = 0; i < BULLETCOUNT; i++)
  {
    Bullet *bullet = &world->bullets[i];

    if (bullet->owner_id == NO_OWNER)
      continue;
    double bullet_orig_x = bullet->x;
    double bullet_orig_y = bullet->y;
    for (int j = 0; j < 12; j++)
    {
      move_bullet(bullet, world);
      if (bullet->owner_id == NO_OWNER)
      {
        if (world->playcount == 0)
          trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
        world->playcount = PLAYDELAY;
        create_explosion(bullet->x - bullet->dx * 5, bullet->y - bullet->dy * 5, world);
        if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
        {
          bullet->x = ((int)(bullet_orig_x / TILESIZE)) * TILESIZE + HALFTILESIZE;
          bullet->y = ((int)(bullet_orig_y / TILESIZE)) * TILESIZE + HALFTILESIZE;
          while (get_tile_at(world, (int)bullet->x, (int)bullet->y)->is_wall)
          {
            bullet->x -= 5 * bullet->dx;
            bullet->y -= 5 * bullet->dy;
          }
          create_cluster_explosion(world, bullet->x, bullet->y, 16, world->powerups.cluster_strength, &world->plr);
        }

        break;
      }
      if ((bullet->hurts_flags & BULLET_HURTS_PLAYER) && bullet_hit(&world->plr, world->bullets + i)) // Player gets hit
      {
        if (no_player_damage)
          world->plr.health++;
        if (world->powerups.rune_of_protection_active == 1)
        {
          world->plr.health++;
          if (world->plr.health < 0)
            world->plr.health = 1;
          world->plr.id = world->plr.former_id;
          world->powerups.rune_of_protection_active = -50;
          create_cluster_explosion(world, world->plr.x, world->plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, &world->plr);
          if ((world->game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0)
          {
            create_cluster_explosion(world, world->plr.x, world->plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, &world->plr);
          }
        }
        if (world->playcount == 0)
          trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
        world->playcount = PLAYDELAY;
        create_shade_around_hit_point(world->plr.x, world->plr.y, 9, world);
        create_explosion(world->plr.x, world->plr.y, world);
        if (world->plr.health <= 0)
        {
          create_explosion(world->plr.x - 20, world->plr.y - 20, world);
          create_explosion(world->plr.x + 20, world->plr.y + 20, world);
          create_explosion(world->plr.x - 20, world->plr.y + 20, world);
          create_explosion(world->plr.x + 20, world->plr.y - 20, world);

          wait_delay_ms(1); // The death sample won't play else
          trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (world->plr.x - 240) / 8, 900 + rand() % 200);
          world->plr.reload = 100;
          break;
        }
      }
      int deathsample_plays = 0;
      if (bullet->hurts_flags & BULLET_HURTS_MONSTERS)
      {
        for (int j = 0; j < ENEMYCOUNT; j++)
        {
          Enemy *enm = &world->enm[j];
          if (enm->id == NO_OWNER || enm->turret == TURRET_TYPE_PLAYER || enm->roomid != world->current_room)
            continue;

          if (bullet_hit(world->enm + j, world->bullets + i))
          {
            create_shade_around_hit_point(enm->x, enm->y, 9, world);
            create_explosion(enm->x, enm->y, world);
            create_explosion(enm->x, enm->y, world);
            create_explosion(enm->x, enm->y, world);
            if (!deathsample_plays)
              if (world->playcount == 0)
                trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);

            if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
            {
              create_cluster_explosion(world, bullet->x, bullet->y, 16, world->powerups.cluster_strength, &world->plr);
            }

            world->playcount = PLAYDELAY;
            if (enm->id == NO_OWNER) // enemy was killed (bullet_hit has side effects)
            {
              
              if (check_potion_effect(world, POTION_EFFECT_STOP_ENEMIES))
              {
                world->potion_effect_flags &= ~POTION_EFFECT_STOP_ENEMIES;
                if (!world->potion_effect_flags)
                  world->potion_duration = 0;
              }

              get_tile_at(world, enm->x, enm->y)->is_blood_stained = 1;
              world->plr.gold += enm->gold;

              world->kills++;
              world->boss_fight_config->state.player_kills++;
              if (enm->gold > 0 || (world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT))
              {
                if (world->game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
                  sprintf(world->hint.text, "%d", world->kills);
                else
                  sprintf(world->hint.text, "+ %d", enm->gold);
                world->hint.loc.x = enm->x - 15;
                world->hint.loc.y = enm->y - 15;
                world->hint.dim = 6;
                world->hint.time_shows = 40;
              }
              if ((world->game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD) == 0)
              {
                if (world->plr.health < 3 && world->plr.health > 0)
                  world->plr.health++;
                world->plr.ammo += 7;
                if (world->plr.ammo > 15)
                  world->plr.ammo = 15;
              }

              if (enm == world->boss) // Archmage dies
              {
                LOG_TRACE("boss die logic\n");
                boss_logic(world, 1);
                stop_all_samples();
                trigger_sample_with_params(SAMPLE_BOSSTALK_2, 255, 127 + (enm->x - 240) / 8, 1000);
                for (int xx = 0; xx < 40; xx++)
                {
                  int col = xx % 4;
                  col = col == 0 ? 255 : (col == 2 ? 64 : 128);
                  rectfill(0, 0, SCREEN_W, SCREEN_H, GRAY(col));
                  wait_delay_ms(25);
                }
                create_cluster_explosion(world, enm->x, enm->y, 48, 1, &world->plr);
              }
              else
              {
                if (!deathsample_plays)
                  trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (enm->x - 240) / 8, 900 + rand() % 200);
              }
              deathsample_plays = 1;

              spawn_body_parts(enm);
            }
            break;
          }
        }
      }
    }
    if (bullet->bullet_type == BULLET_TYPE_NORMAL)
    {
      int bullet_sprite = ((int)(bullet->x + bullet->y) / 30) % 4;
      draw_sprite_animated_centered(world->spr, SPRITE_ID_BULLET, bullet->x, bullet->y, bullet_sprite, 0);
    }
    else if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
    {
      draw_sprite_centered(world->spr, SPRITE_ID_CLUSTER, bullet->x, bullet->y);
    }
  }
}

void potion_logic(World *w)
{
  static unsigned potion_anim_phase = 0;
  for (int i = 0; i < POTION_COUNT; i++)
  {
    Potion *p = &w->potions[i];
    if (p->exists && p->room_id == w->current_room)
    {
      if (w->plr.x > p->location.x - 20 && w->plr.x < p->location.x + 20 &&
          w->plr.y > p->location.y - 20 && w->plr.y < p->location.y + 20)
      {
        p->exists = 0;
        w->potion_duration += p->duration_boost;
        if (w->potion_duration > POTION_DURATION_CAP)
          w->potion_duration = POTION_DURATION_CAP;
        w->potion_effect_flags |= p->effects;
        trigger_sample(p->sample, 255);
        create_sparkles(p->location.x, p->location.y, 12, 1, w);
      }

      unsigned anim_phase = (potion_anim_phase + i * 1337) % 200;
      if (anim_phase < 30)
      {
        int bubble_y = anim_phase / 5;
        int bubble_x = 4 * sin(bubble_y);
        int pop_spr = anim_phase > 25;
        draw_sprite_animated_centered(w->spr, SPRITE_ID_POTION_BUBBLE, p->location.x + bubble_x, p->location.y - 13 - bubble_y, p->sprite * 2 + pop_spr, 0);
      }
      draw_sprite_animated_centered(w->spr, SPRITE_ID_POTION, p->location.x, p->location.y, p->sprite, 0);
    }
  }
  potion_anim_phase++;
  if (w->potion_duration > 0)
  {
    w->potion_duration--;
  }
  else
  {
    w->potion_effect_flags = 0;
  }
}

int game(int mission, int *game_modifiers)
{
  pr_reset_random();

  long completetime = 0;

  World world;
  world.game_modifiers = *game_modifiers;
  world.mission = mission;
  memset(&world.boss_fight_configs, 0, sizeof(world.boss_fight_configs));
  world.boss_fight_config = world.boss_fight_configs;
  if (!game_settings.custom_resources)
  {
    world.spr = al_load_bitmap(DATADIR "sprites.png");
  }
  else
  {
    char path[256];
    sprintf(path, DATADIR "\\%s\\sprites.png", game_settings.mission_pack);
    world.spr = al_load_bitmap(path);
  }
  al_convert_mask_to_alpha(world.spr, al_map_rgb(255, 0, 255));

  char c;
  int vibrations = 0;
  int x, y, i, additt_anim = 0;
  world.playcount = 0;

  int fly_in_text_x = SCREEN_W;
  char fly_in_text[64];
  strcpy(fly_in_text, world.mission_display_name);
  int boss_fight_frame_count = 0;
  //world.boss_want_to_shoot = 0;

  world.current_room = 1;

  init_world(&world);
  read_enemy_configs(&world);
  init_player(&world, &plrautosave);

  FILE *f_key_presses = NULL;
  ContinuousData key_presses[128];
  int key_press_buffer_sz = 128;
  int key_press_buffer_idx = 0;
  long key_press_mask = 0;

  //world.boss_waypoint.x = world.boss_waypoint.y = -1;
  world.hint.time_shows = 0;

  if (record_mode == RECORD_MODE_RECORD)
  {
    char fname[100];
    sprintf(fname, "recorded-mission%d-take%d.dat", mission, ++fname_counter);
    save_game_save_data(record_input_filename, &world.plr, mission, *game_modifiers, 0);
    f_key_presses = fopen(fname, "w");
    fprintf(f_key_presses, "\n-- data start --\n");
  }
  else if (record_mode == RECORD_MODE_PLAYBACK)
  {
    fseek(record_input_file, 0, SEEK_SET);
    f_key_presses = record_input_file;
    load_game_save_data(record_input_filename, &world.plr, &mission, &world.game_modifiers, 0);
    key_press_buffer_idx = -1;
  }
  long time_stamp = 0;

  read_level(&world, mission, 1);

  if (world.boss_fight && world.play_boss_sound)
  {
    trigger_sample_with_params(SAMPLE_BOSSTALK_1, 255, 127, 1000);
  }

  int difficulty = GET_DIFFICULTY(&world);
  int excess_gold_limit = difficulty == DIFFICULTY_BRUTAL ? 0 : 5;
  if (world.game_modifiers & GAMEMODIFIER_OVERPRICED_POWERUPS)
  {
    excess_gold_limit = difficulty == DIFFICULTY_BRUTAL ? 2 : 7;
  }
  if (world.game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD)
  {
    excess_gold_limit = 0;
  }

  if (world.plr.gold > excess_gold_limit)
  {
    int excess_gold = world.plr.gold - (difficulty == DIFFICULTY_BRUTAL ? 0 : 5);
    if (world.game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD)
    {
      excess_gold = world.plr.gold;
    }
    if (world.game_modifiers & GAMEMODIFIER_OVERPRICED_POWERUPS)
      excess_gold /= 3;
    world.plr.health += excess_gold * (difficulty == DIFFICULTY_BRUTAL ? 2 : 3);
    world.plr.health = world.plr.health > 6 ? 6 : world.plr.health;

    if (world.game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS)
    {
      world.plr.health *= 3;
    }
  }

  world.plr.gold = world.plr.gold > 5 ? 5 : world.plr.gold;

  if (world.plr.health < 3)
    world.plr.health = 3;
  if (world.plr.ammo < 10)
    world.plr.ammo = 10;

  world.powerups.cluster_strength = 16;

  if ((world.game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD) != 0)
  {
    world.powerups.cluster_strength = 5;
    world.plr.gold = 20;
  }
  else if (difficulty == DIFFICULTY_BRUTAL)
    world.plr.gold = 0;

  if (world.boss_fight && world.boss_fight_config->player_initial_gold >= 0)
  {
    world.plr.gold = world.boss_fight_config->player_initial_gold;
  }

  world.powerups.rune_of_protection_active = 0;

  int plr_dir_helper_intensity = 0;

  int restart_requested = 0;

  int screen_width_scaled, screen_h_offset, screen_v_offset, screen_height_scaled;
  {
    double screen_ratio = 480.0 / 360.0;
    screen_width_scaled = SCREEN_H * screen_ratio;
    screen_height_scaled = SCREEN_H;
    if (screen_width_scaled > SCREEN_W)
    {
      screen_width_scaled = SCREEN_W;
      screen_height_scaled = SCREEN_W / screen_ratio;
    }
    screen_h_offset = (SCREEN_W - screen_width_scaled) / 2;
    screen_v_offset = (SCREEN_H - screen_height_scaled) / 2;
  }

  draw_static_background();

  create_sparkles(world.plr.x, world.plr.y, 30, -1, &world);

  clock_t game_loop_clk = clock();

  while (restart_requested < 2)
  {
    if (world.plr.health <= 0)
    {
      // Draw well outside of screen so that the zoom in transformation would not look like ass
      rectfill(0, 0, SCREEN_W * 2, SCREEN_H * 2, BLACK);
    }
    cleanup_bodyparts(&world);
    if (time_stamp % 6 == 0)
    {
      reset_sample_triggers();
    }
    time_stamp++;
    draw_map(&world, 0, vibrations); // shadows
    move_and_draw_body_parts(&world);

    if (world.playcount > 0)
      world.playcount--;

    vibrations = progress_and_draw_explosions(&world);
    if (game_settings.vibration_mode != 1)
    {
      if (game_settings.vibration_mode == 0)
      {
        vibrations = 0;
      }
      else
      {
        vibrations /= game_settings.vibration_mode;
      }
    }

    // Draw legend to same position as player
    // Legend cannot be drawn here or it would get obscured
    // by walls etc.
    int legend_x = world.plr.x;
    int legend_y = world.plr.y;
    if (world.plr.health > 0)
    {
      draw_enemy(&world.plr, &world);

      int key_left, key_right, key_up, key_down, key_space,
          key_x, key_z, key_a, key_s, key_d, key_f;
      if (record_mode == RECORD_MODE_PLAYBACK)
      {
        int has_more = read_and_process_continuous_data(key_presses,
                                                        &key_press_buffer_sz, &key_press_buffer_idx, f_key_presses,
                                                        time_stamp, get_key_presses, &key_press_mask);
        if (!has_more)
        {
          world.hint.time_shows = 0;
          trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

          display_level_info(&world, mission, game_settings.mission_count, completetime);

          wait_key_press(ALLEGRO_KEY_ENTER);
          break;
        }

        key_left = key_press_mask & 1;
        key_right = key_press_mask & 2;
        key_up = key_press_mask & 4;
        key_down = key_press_mask & 8;
        key_space = key_press_mask & 16;
        key_x = key_press_mask & 32;
        key_z = key_press_mask & 64;
        key_a = key_press_mask & 128;
        key_s = key_press_mask & 256;
        key_d = key_press_mask & 512;
        key_f = key_press_mask & 1024;
      }
      else
      {
        key_left = check_key(ALLEGRO_KEY_LEFT);
        key_right = check_key(ALLEGRO_KEY_RIGHT);
        key_up = check_key(ALLEGRO_KEY_UP);
        key_down = check_key(ALLEGRO_KEY_DOWN);
        key_space = check_key(ALLEGRO_KEY_SPACE);
        key_x = check_key(ALLEGRO_KEY_X);
        key_z = check_key(ALLEGRO_KEY_Z);
        key_a = check_key(ALLEGRO_KEY_A);
        key_s = check_key(ALLEGRO_KEY_S);
        key_d = check_key(ALLEGRO_KEY_D);
        key_f = check_key(ALLEGRO_KEY_F);
      }

      int player_did_change_dir = handle_direction_keys(&world, key_up, key_down, key_left, key_right);
      if (player_did_change_dir)
      {
        plr_dir_helper_intensity = PLR_DIR_HELPER_INITIAL_INTENSITY;
      }

      int play_sample = handle_shoot_key(&world, key_space);

      if (play_sample)
      {
        trigger_sample(SAMPLE_THROW, 255);
      }

      play_sample = handle_weapon_change_keys(&world, key_x, key_z);
      if (play_sample)
      {
        trigger_sample_with_params(SAMPLE_SELECT_WEAPON, 127, 127, 1000);
      }

      int gold_hint_amount = 0;
      int activated_powerup = handle_power_up_keys(&world, key_a, key_s, key_d, key_f, &gold_hint_amount);
      play_sample = activated_powerup || play_sample;
      if (gold_hint_amount)
      {
        show_gold_hint(&world, gold_hint_amount);
      }

      if (check_key(ALLEGRO_KEY_R))
      {
        restart_requested = 1;
      }
      else if (restart_requested)
        restart_requested = 2;

      if (record_mode == RECORD_MODE_RECORD)
      {
        long new_key_press_mask = (key_left ? 1 : 0) | (key_right ? 2 : 0) |
                                  (key_up ? 4 : 0) | (key_down ? 8 : 0) |
                                  (key_space ? 16 : 0) | (key_x ? 32 : 0) |
                                  (key_z ? 64 : 0) | (key_a ? 128 : 0) |
                                  (key_s ? 256 : 0) | (key_d ? 512 : 0) |
                                  (key_f ? 1024 : 0);
        if (new_key_press_mask != key_press_mask)
        {
          key_press_mask = new_key_press_mask;
          produce_and_write_continuous_data(key_presses, key_press_buffer_sz, &key_press_buffer_idx, f_key_presses, time_stamp, 1, key_press_mask);
        }
      }
    }
    else
    {
      world.plr.move = 0;
    }
    for (int plr_speed = check_potion_effect(&world, POTION_EFFECT_FAST_PLAYER) ? 4 : 3; plr_speed >= 0; plr_speed--)
    {
      move_enemy(&world.plr, &world);
    }

    change_room_if_at_exit_point(&world, mission);

    if (get_tile_at(&world, world.plr.x, world.plr.y)->is_exit_level && world.plr.health > 0)
    {
      world.hint.time_shows = 0;
      trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

      display_level_info(&world, mission, game_settings.mission_count, completetime);

      wait_key_press(ALLEGRO_KEY_ENTER);

      if (world.final_level)
      {
        mission = -1;
        break;
      }

      mission++;
      plrautosave = world.plr;
      break;
    }

    if (world.plr.health > 0 && world.plr.health < 6 && check_potion_effect(&world, POTION_EFFECT_HEALING) && completetime % 25 == 0)
    {
      world.plr.health++;
    }

    if (world.plr.health > 0 && check_potion_effect(&world, POTION_EFFECT_SHIELD_OF_FIRE) && completetime % 15 == 0)
    {
      create_cluster_explosion(&world, world.plr.x, world.plr.y, 4, 1, &world.plr);
    }

    enemy_logic(&world);

    if (world.plr.health > 0)
    {
      bullet_logic(&world);
      potion_logic(&world);
    }

    draw_map(&world, 1, 0);

    draw_player_legend(&world, legend_x, legend_y);

    completetime++;

    // Draw hint

    if (world.hint.time_shows > 0)
    {
      world.hint.time_shows--;
      int hint_col = world.hint.time_shows * world.hint.dim;
      al_draw_textf(get_font(), GRAY(hint_col), world.hint.loc.x, world.hint.loc.y, -1, world.hint.text);
    }
    if (world.boss_fight && ++boss_fight_frame_count >= 3)
    {
      boss_fight_frame_count = 0;
      boss_logic(&world, 0);
    }

    if (world.boss && world.boss->roomid == world.current_room)
    {
      for (int i = 0; i < 6; i++)
      {
        if (world.boss->health >= (world.boss_fight_config->health * (i + 1) / 6))
          draw_sprite(world.spr, SPRITE_ID_HEALTH, world.boss->x - 23, world.boss->y - 18 + 4 * i);
      }
    }

    if (plr_dir_helper_intensity > 0)
    {
      al_draw_circle(world.plr.x + world.plr.dx * TILESIZE * 3 / 2,
                     world.plr.y + world.plr.dy * TILESIZE * 3 / 2,
                     plr_dir_helper_intensity * TILESIZE / 600,
                     al_map_rgb(2 * plr_dir_helper_intensity, 0, 0), 1);
      plr_dir_helper_intensity -= 3;
    }

    if (world.powerups.rune_of_protection_active)
    {
      if (world.powerups.rune_of_protection_active < 0)
      {
        world.powerups.rune_of_protection_active++;
        draw_sprite_centered(world.spr, SPRITE_ID_RUNE_OF_PROTECTION, 
          world.plr.x + world.powerups.rune_of_protection_active * sin(completetime * 0.15),
          world.plr.y + world.powerups.rune_of_protection_active * cos(completetime * 0.15));
      }
      else
      {
        draw_sprite_centered(world.spr, SPRITE_ID_RUNE_OF_PROTECTION, 
          world.plr.x - TILESIZE * sin(completetime * 0.15),
          world.plr.y - TILESIZE * cos(completetime * 0.15));
      }
    }

    progress_and_draw_sparkles(&world);

    if (fly_in_text_x > -400)
    {
      al_draw_textf(get_font(), WHITE, fly_in_text_x, 170, -1, world.mission_display_name);
      if (fly_in_text_x > SCREEN_W / 8 * 3 && fly_in_text_x < SCREEN_W / 8 * 5)
      {
        fly_in_text_x -= 4;
      }
      else
      {
        fly_in_text_x -= 10;
      }
    }

    if (world.plr.health > 0)
    {
      int offset_x = 2 * vibrations - rand() % (1 + 2 * vibrations);
      int offset_y = 2 * vibrations - rand() % (1 + 2 * vibrations);
      ALLEGRO_TRANSFORM transform;
      al_identity_transform(&transform);

      al_translate_transform(&transform, offset_x, offset_y);
      al_scale_transform(&transform, 3, 3);
      al_use_transform(&transform);
      al_flip_display();
      play_mp3();
    }
    else
    {
      int startx, starty;
      startx = world.plr.x - TILESIZE * 3 / 2 - world.plr.reload;
      if (startx < 0)
        startx = 0;
      starty = world.plr.y - TILESIZE - world.plr.reload * 0.75;
      if (starty < 0)
        starty = 0;
      double scale = 1 + (100 - world.plr.reload) / 20.0;
      ALLEGRO_TRANSFORM transform;
      al_identity_transform(&transform);
      al_translate_transform(&transform, -startx, -starty);
      al_scale_transform(&transform, 3 * scale, 3 * scale);
      al_use_transform(&transform);
      al_flip_display();
      wait_delay_ms(40);
      if (world.plr.reload <= 0)
      {
        al_identity_transform(&transform);
        al_use_transform(&transform);
        if (world.game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
        {
          ArenaHighscore highscore;
          memset(&highscore, 0, sizeof(highscore));
          access_arena_highscore(&highscore, 1);
          int arena_idx, mode_idx;
          int highscore_kills = parse_highscore_from_world_state(&world, &highscore, &arena_idx, &mode_idx);
          int offx = (DISPLAY_W - 340) / 2, offy = (DISPLAY_H - 125) / 2;
          for (int grayscale = 0; grayscale < 5; grayscale++)
          {
            rectfill(offx + grayscale, offy + grayscale, offx + 345 - grayscale, offy + 130 - grayscale, GRAY(255 - grayscale * 40));
          }
          rectfill(offx + 5, offy + 5, offx + 340, offy + 125, GRAY(60));
          al_draw_textf(get_font(), WHITE, offx + 10, offy + 10, ALLEGRO_ALIGN_LEFT, "Arena fight over, your kill count: %d", world.kills);
          if (record_mode == RECORD_MODE_NONE && highscore_kills < world.kills)
          {
            al_draw_textf(get_font(), WHITE, offx + 10, offy + 30, ALLEGRO_ALIGN_LEFT, "Previous highscore: %d", highscore_kills);
            al_draw_textf(get_font(), WHITE, offx + 10, offy + 50, ALLEGRO_ALIGN_LEFT, "NEW HIGHSCORE!");
            highscore.kills[arena_idx][mode_idx] = world.kills;
            if (!no_player_damage)
            {
              LOG("Not saving highscore, no damage mode active");
              access_arena_highscore(&highscore, 0);
            }
          }
          else
          {
            al_draw_textf(get_font(), WHITE, offx + 10, offy + 30, ALLEGRO_ALIGN_LEFT, "Highscore: %d", highscore_kills);
          }
          al_draw_textf(get_font(), WHITE, offx + 10, offy + 100, ALLEGRO_ALIGN_LEFT, "Press ENTER to continue...");
          al_flip_display();
          int wait_keys[] = {ALLEGRO_KEY_ENTER, ALLEGRO_KEY_ESCAPE};
          int key = wait_key_presses(wait_keys, 2);
          if (key == ALLEGRO_KEY_ESCAPE)
          {
            menu(0, &plrautosave, &mission, game_modifiers);
          }
        }
        break;
      }
    }

    game_loop_rest(&game_loop_clk);

    if (check_key(ALLEGRO_KEY_ESCAPE))
    {

      world.hint.time_shows = 0;
      int switch_level = menu(1, &plrautosave, &mission, game_modifiers);
      if (switch_level)
      {
        break;
      }
      draw_static_background();
    }
  }

  if (record_mode == RECORD_MODE_RECORD)
  {
    produce_and_write_continuous_data(key_presses, key_press_buffer_idx + 1, &key_press_buffer_idx, f_key_presses, time_stamp, 999, 999);
    fclose(f_key_presses);
  }

  al_destroy_bitmap(world.spr);

  {
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_use_transform(&transform);
  }
  return mission;
}

/*void init_allegro()
{
  srand((int)time(NULL));
  allegro_init();
  install_timer();

  set_color_depth(32);
  install_keyboard();

  if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
  {
    allegro_message("Error initialising sound system.\n%s\n", allegro_error);
    exit(1);
  }

  int w = game_settings.screen_width;
  int h = game_settings.screen_height;
  int full_screen = game_settings.screen_mode == 1;
  if (full_screen)
  {
    LOG("starting in fullscreen\n");
    full_screen = !set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, w, h, 0, 0);
  }
  if (!full_screen)
  {
    if (set_gfx_mode(GFX_AUTODETECT_WINDOWED, 960, 720, 0, 0))
    {
      set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
      allegro_message("Unable to set graphics mode.\n%s\n", allegro_error);
      exit(1);
    }
  }
  LOG("allegro inited\n");
}*/
