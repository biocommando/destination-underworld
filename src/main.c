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
#include "iniRead.h"
#include "bossfightconf.h"
#include "predictableRandom.h"
#include "settings.h"
#include "continuousData.h"
#include "keyhandling.h"
#include "sampleRegister.h"
#include "helpers.h"

extern MP3FILE *mp3;

void init_allegro();
int game(int mission, int *game_modifiers);
void read_settings();

SAMPLE *s_throw;

GameSettings game_settings;

Enemy plrautosave;

FONT *GameFont;
int fname_counter = 0;

int record_mode = RECORD_MODE_NONE;
FILE *record_input_file = NULL;

int main(int argc, char **argv)
{
  record_mode = read_cmd_line_arg_int("record-mode", argv, argc);
  if (record_mode == RECORD_MODE_RECORD)
  {
    LOG("Record mode active.\n");
  }
  if (record_mode == RECORD_MODE_PLAYBACK)
  {
    char fname[256];
    if (read_cmd_line_arg_str("file", argv, argc, fname))
    {
      record_input_file = fopen(fname, "r");
    }
    if (record_input_file == NULL)
    {
      LOG("Valid input file required (--file=<filename>)!!\n");
      return 0;
    }
    LOG("Playback mode active.\n");
  }

  read_settings();
  init_allegro();
  srand((int)time(NULL));
  int mission = 1;
  int game_modifiers = 0;
  GameFont = load_font(FONT_FILENAME, default_palette, NULL);

  register_sample(SAMPLE_SELECT, "sel.wav");
  register_sample(SAMPLE_WARP, "warp.wav");
  register_sample(SAMPLE_BOSSTALK_1, "bt1.wav");
  register_sample(SAMPLE_BOSSTALK_2, "bt2.wav");
  register_sample(SAMPLE_THROW, "throw.wav");
  register_sample(SAMPLE_SELECT_WEAPON, "select_weapon.wav");
  register_sample(SAMPLE_HEAL, "healing.wav");
  register_sample(SAMPLE_PROTECTION, "rune_of_protection.wav");
  register_sample(SAMPLE_TURRET, "turret.wav");
  register_sample(SAMPLE_SPAWN, "spawn.wav");

  for (int i = 0; i < 6; i++)
  {
    char loadsamplename[100];
    sprintf(loadsamplename, "ex%d.wav", i + 1);
    register_sample(SAMPLE_EXPLOSION(i), loadsamplename);
    sprintf(loadsamplename, "die%d.wav", i + 1);
    register_sample(SAMPLE_DEATH(i), loadsamplename);
  }

  play_track(1);
  menu(0, &plrautosave, &mission, &game_modifiers);

  while (mission != 0)
  {
    mission = game(mission, &game_modifiers);
    if (record_input_file)
      break;
  }
  destroy_registered_samples();
  if (record_input_file)
    fclose(record_input_file);

  close_mp3_file(mp3);
  destroy_font(GameFont);
  set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
  remove_keyboard();
  remove_mouse();
  return 0;
}
END_OF_MAIN();

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
{ // Viholliset
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

        if (reacts_to_player || (is_boss && world->boss_waypoint.x >= 0))
        {
          enm->move = 1;
          if (!is_boss || (world->boss_want_to_shoot && reacts_to_player))
          {
            if (is_boss)
            {
              set_directions(enm, &aim_at, aim_window);
            }
            int play_sample = shoot(enm, world);
            if (play_sample)
            {
              trigger_sample(SAMPLE_THROW, 255);
            }
          }

          enm->dx = enm->dy = 0;
          if (is_boss && world->boss_waypoint.x >= 0)
          {
            aim_at.x = world->boss_waypoint.x * TILESIZE + HALFTILESIZE;
            aim_at.y = world->boss_waypoint.y * TILESIZE + HALFTILESIZE;
            aim_window = 0;
            if (enm->x / TILESIZE == (int)world->boss_waypoint.x && enm->y / TILESIZE == (int)world->boss_waypoint.y)
            {
              LOG("Waypoint reached\n");
              world->boss_waypoint.x = world->boss_waypoint.y = -1;
              world->boss_fight_config.state.waypoint_reached = 1;
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
          for (int m = 0; m < world->boss_fight_config.speed; m++)
            move_enemy(enm, world);
        }
        else if (enm->turret == TURRET_TYPE_NONE)
        {
          if (enm->fast)
          {
            move_enemy(enm, world);
          }
          move_enemy(enm, world);
        }
        else if (enm->reload > 0)
        {
          enm->reload--;
        }
        // if (enm_type != ALIEN || rand() % 32) // Alienit (muttei turretit) vilkkuvat
        //{
        draw_enemy(enm, world);
        //}
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
  rectfill(screen, 0, 0, screen->w, screen->h, 0);
  int maxsz = screen->h > screen->w ? screen->h : screen->w;
  for (int i = 0; i < maxsz / 2; i += maxsz / 80)
  {
    circle(screen,
           screen->w / 2,
           screen->h / 2,
           i,
           makecol(33, 33, 33));
  }
}

void boss_logic(World *world, int boss_died)
{
  Enemy *boss = world->boss;
  int in_same_room = boss != NULL && boss->roomid == world->current_room;
  if (in_same_room || boss_died)
  {
    world->boss_fight_config.state.health = boss_died ? 0 : boss->health;
    world->boss_fight_config.state.player_kills = world->kills;
    bossfight_process_event_triggers(&world->boss_fight_config);
    for (int x = 0; x < world->boss_fight_config.num_events; x++)
    {
      if (!world->boss_fight_config.state.triggers[x])
        continue;

      BossFightEventConfig *event = &world->boss_fight_config.events[x];

      char s[100];
      bossfight_event_type_to_str(s, event->event_type);
      LOG("Trigger %s\n", s);
      if (!event->enabled)
      {
        LOG("Event disabled\n");
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
            create_sparkles(spawn_point->x * TILESIZE + HALFTILESIZE, spawn_point->y * TILESIZE + HALFTILESIZE, 15, world);

            trigger_sample(SAMPLE_SPAWN, 255);
            break;
          }
        }
      }
      break;
      case BFCONF_EVENT_TYPE_ALLOW_FIRING:
        world->boss_want_to_shoot = 1;
        break;
      case BFCONF_EVENT_TYPE_DISALLOW_FIRING:
        world->boss_want_to_shoot = 0;
        break;
      case BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE:
        if (boss)
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
          world->map[event->parameters[0]][event->parameters[1]] = create_tile(tile_type);
          trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
          for (int y = 0; y < 3; y++)
            create_explosion(event->parameters[0] * TILESIZE + TILESIZE / 2, event->parameters[1] * TILESIZE + TILESIZE / 2, world);
        }
      }
      break;
      case BFCONF_EVENT_TYPE_SET_WAYPOINT:
        world->boss_waypoint.x = event->parameters[0];
        world->boss_waypoint.y = event->parameters[1];
        world->boss_fight_config.state.waypoint = event->parameters[2];
        world->boss_fight_config.state.waypoint_reached = 0;
        break;
      case BFCONF_EVENT_TYPE_CLEAR_WAYPOINT:
        world->boss_waypoint.x = -1;
        world->boss_waypoint.y = -1;
        world->boss_fight_config.state.waypoint = 0;
        break;
      case BFCONF_EVENT_TYPE_START_SECONDARY_TIMER:
        world->boss_fight_config.state.secondary_timer_started = 1;
        if (event->parameters[0] >= 0)
          world->boss_fight_config.state.secondary_timer_value = event->parameters[0];
        break;
      case BFCONF_EVENT_TYPE_STOP_SECONDARY_TIMER:
        world->boss_fight_config.state.secondary_timer_started = 0;
        break;
      case BFCONF_EVENT_TYPE_TOGGLE_EVENT_ENABLED:
        world->boss_fight_config.events[event->parameters[0]].enabled = event->parameters[1];
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
          while (check_flags_at(world, (int)bullet->x, (int)bullet->y, TILE_IS_WALL))
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

          chunkrest(1); // The death sample won't play else
          trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (world->plr.x - 240) / 8, 900 + rand() % 200);
          world->plr.reload = 100;
          break;
        }
      }
      int deathsample_plays = 0;
      if (bullet->hurts_flags & BULLET_HURTS_MONSTERS)
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
              set_tile_flag(world, enm->x, enm->y, TILE_IS_BLOOD_STAINED);
              world->plr.gold += enm->gold;

              world->kills++;
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
                LOG("boss die logic\n");
                boss_logic(world, 1);
                chunkrest(1);
                trigger_sample_with_params(SAMPLE_BOSSTALK_2, 255, 127 + (enm->x - 240) / 8, 1000);
              }
              else
              {
                chunkrest(1); // The death sample won't play else
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
    if (bullet->bullet_type == BULLET_TYPE_NORMAL)
    {
      int bullet_sprite = ((int)(bullet->x + bullet->y) / 30) % 4;
      masked_blit(world->spr, world->buf, 140 + bullet_sprite * 10, 140, bullet->x - 5, bullet->y - 5, 10, 10);
    }
    else if (bullet->bullet_type == BULLET_TYPE_CLUSTER)
    {
      masked_blit(world->spr, world->buf, 140, 150, bullet->x - 6, bullet->y - 6, 13, 12);
    }
  }
}

int game(int mission, int *game_modifiers)
{
  pr_reset_random();

  long completetime = 0;

  World world;
  world.game_modifiers = *game_modifiers;
  world.mission = mission;
  memset(&world.boss_fight_config, 0, sizeof(BossFightConfig));
  world.buf = create_bitmap(480, 360);
  if (!game_settings.custom_resources)
  {
    world.spr = load_bitmap(".\\dataloss\\sprites.bmp", default_palette);
  }
  else
  {
    char path[256];
    sprintf(path, ".\\dataloss\\%s\\sprites.bmp", game_settings.mission_pack);
    world.spr = load_bitmap(path, default_palette);
  }

  char c;
  int vibrations = 0;
  int x, y, i, additt_anim = 0;
  world.playcount = 0;

  int fly_in_text_x = world.buf->w;
  int boss_fight_frame_count = 0;
  world.boss_want_to_shoot = 0;

  world.current_room = 1;

  init_world(&world);
  read_enemy_configs(&world);
  init_player(&world, &plrautosave);

  FILE *f_key_presses = NULL;
  ContinuousData key_presses[128];
  int key_press_buffer_sz = 128;
  int key_press_buffer_idx = 0;
  long key_press_mask = 0;

  world.boss_waypoint.x = world.boss_waypoint.y = -1;
  world.hint.time_shows = 0;

  if (record_mode == RECORD_MODE_RECORD)
  {
    char fname[100];
    sprintf(fname, "recorded-mission%d-take%d.dat", mission, ++fname_counter);
    f_key_presses = fopen(fname, "w");
    save_game_save_data(f_key_presses, &world.plr, mission, *game_modifiers);
    fprintf(f_key_presses, "\n-- data start --\n");
  }
  else if (record_mode == RECORD_MODE_PLAYBACK)
  {
    fseek(record_input_file, 0, SEEK_SET);
    f_key_presses = record_input_file;
    load_game_save_data(f_key_presses, &world.plr, &mission, &world.game_modifiers);
    key_press_buffer_idx = -1;
  }
  long time_stamp = 0;

  read_level(&world, mission, 1);

  if (world.boss_fight && !(world.game_modifiers & GAMEMODIFIER_ARENA_FIGHT))
  {
    trigger_sample_with_params(SAMPLE_BOSSTALK_1, 255, 127, 1000);
  }

  int difficulty = GET_DIFFICULTY(&world); //(world.game_modifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : 0;
  int excess_gold_limit = difficulty == DIFFICULTY_BRUTAL ? 0 : 5;
  if (world.game_modifiers & GAMEMODIFIER_OVERPRICED_POWERUPS)
  {
    excess_gold_limit = difficulty == DIFFICULTY_BRUTAL ? 2 : 7;
  }

  if (world.plr.gold > excess_gold_limit)
  {
    int excess_gold = world.plr.gold - (difficulty == DIFFICULTY_BRUTAL ? 0 : 5);
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
    world.plr.gold = 40;
  }
  else if (difficulty == DIFFICULTY_BRUTAL)
    world.plr.gold = 0;

  if (world.boss_fight && world.boss_fight_config.player_initial_gold >= 0)
  {
    world.plr.gold = world.boss_fight_config.player_initial_gold;
  }

  world.powerups.rune_of_protection_active = 0;

  int plr_dir_helper_intensity = 0;

  int restart_requested = 0;

  int screen_width_scaled, screen_h_offset, screen_v_offset, screen_height_scaled;
  {
    double screen_ratio = 480.0 / 360.0;
    screen_width_scaled = screen->h * screen_ratio;
    screen_height_scaled = screen->h;
    if (screen_width_scaled > screen->w)
    {
      screen_width_scaled = screen->w;
      screen_height_scaled = screen->w / screen_ratio;
    }
    screen_h_offset = (screen->w - screen_width_scaled) / 2;
    screen_v_offset = (screen->h - screen_height_scaled) / 2;
  }

  draw_static_background();

  create_sparkles(world.plr.x, world.plr.y, 30, &world);

  clock_t game_loop_clk = clock();

  while (restart_requested < 2)
  {
    if (time_stamp % 6 == 0)
      reset_sample_triggers();
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

          display_level_info(&world, mission, game_settings.mission_count, font, completetime);

          while (!key[KEY_ENTER])
          {
            chunkrest(15);
          }
          chunkrest(250);
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
        key_left = key[KEY_LEFT];
        key_right = key[KEY_RIGHT];
        key_up = key[KEY_UP];
        key_down = key[KEY_DOWN];
        key_space = key[KEY_SPACE];
        key_x = key[KEY_X];
        key_z = key[KEY_Z];
        key_a = key[KEY_A];
        key_s = key[KEY_S];
        key_d = key[KEY_D];
        key_f = key[KEY_F];
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

      if (key[KEY_R])
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
    move_enemy(&world.plr, &world);
    move_enemy(&world.plr, &world);
    move_enemy(&world.plr, &world);

    change_room_if_at_exit_point(&world, mission);

    if (get_tile_at(&world, world.plr.x, world.plr.y).flags & TILE_IS_EXIT_LEVEL && world.plr.health > 0)
    {
      world.hint.time_shows = 0;
      trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

      display_level_info(&world, mission, game_settings.mission_count, font, completetime);

      while (!key[KEY_ENTER])
      {
        chunkrest(15);
      }
      chunkrest(250);
      mission++;
      plrautosave = world.plr;
      break;
    }

    enemy_logic(&world);

    // ammukset

    if (world.plr.health > 0)
    {
      bullet_logic(&world);
    }

    draw_map(&world, 1, 0);

    draw_player_legend(&world);

    completetime++;

    // Draw hint

    if (world.hint.time_shows > 0)
    {
      world.hint.time_shows--;
      int hint_col = world.hint.time_shows * world.hint.dim;
      textprintf_ex(world.buf, font, world.hint.loc.x, world.hint.loc.y, GRAY(hint_col), -1, world.hint.text);
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
        if (world.boss->health >= (world.boss_fight_config.health * (i + 1) / 6))
          masked_blit(world.spr, world.buf, 60, 0, world.boss->x - 23, world.boss->y - 18 + 4 * i, 7, 6);
      }
    }

    if (plr_dir_helper_intensity > 0)
    {
      circle(world.buf,
             world.plr.x + world.plr.dx * TILESIZE * 3 / 2,
             world.plr.y + world.plr.dy * TILESIZE * 3 / 2,
             plr_dir_helper_intensity * TILESIZE / 600,
             makecol(2 * plr_dir_helper_intensity, 0, 0));
      plr_dir_helper_intensity -= 3;
    }

    if (world.powerups.rune_of_protection_active)
    {
      if (world.powerups.rune_of_protection_active < 0)
      {
        world.powerups.rune_of_protection_active++;
        masked_blit(world.spr, world.buf, 140, 165,
                    world.plr.x + world.powerups.rune_of_protection_active * sin(completetime * 0.15) - 7,
                    world.plr.y + world.powerups.rune_of_protection_active * cos(completetime * 0.15) - 7,
                    13, 13);
      }
      else
        masked_blit(world.spr, world.buf, 140, 165,
                    world.plr.x - TILESIZE * sin(completetime * 0.15) - 7,
                    world.plr.y - TILESIZE * cos(completetime * 0.15) - 7,
                    13, 13);
    }

    progress_and_draw_sparkles(&world);

    if (fly_in_text_x > -400)
    {
      textprintf_ex(world.buf, font, fly_in_text_x, 170, GRAY(255), -1, world.mission_display_name);
      if (fly_in_text_x > world.buf->w / 8 * 3 && fly_in_text_x < world.buf->w / 8 * 5)
      {
        fly_in_text_x -= 4;
      }
      else
      {
        fly_in_text_x -= 10;
      }
    }

    int offset = 2 * vibrations - rand() % (1 + 2 * vibrations);
    if (world.plr.health > 0)
    {
      stretch_blit(world.buf, screen, 0, 0, 480, 360, offset + screen_h_offset, offset + screen_v_offset, screen_width_scaled, screen_height_scaled);
    }
    else
    {
      int startx, starty, endx, endy;
      startx = world.plr.x - world.plr.reload;
      if (startx < 0)
        startx = 0;
      starty = world.plr.y - world.plr.reload * 0.75;
      if (starty < 0)
        starty = 0;
      endx = world.plr.reload * 2;
      if (startx + endx > 480)
        endx = 480 - startx;
      endy = world.plr.reload * 2 * 0.75;
      if (starty + endy > 360)
        endy = 360 - starty;
      stretch_blit(world.buf, screen, startx, starty, endx, endy, screen_h_offset, screen_v_offset, screen_width_scaled, screen_height_scaled);
      chunkrest(40);
      if (world.plr.reload <= 0)
      {
        if (world.game_modifiers & GAMEMODIFIER_ARENA_FIGHT)
        {
          ArenaHighscore highscore;
          access_arena_highscore(&highscore, 1);
          int arena_idx, mode_idx;
          int highscore_kills = parse_highscore_from_world_state(&world, &highscore, &arena_idx, &mode_idx);
          rectfill(screen, 5, 5, 340, 125, GRAY(60));
          textprintf_ex(screen, font, 10, 10, WHITE, -1, "Arena fight over, your kill count: %d", world.kills);
          textprintf_ex(screen, font, 10, 30, WHITE, -1, "Previous highscore: %d", highscore_kills);
          if (highscore_kills < world.kills)
          {
            textprintf_ex(screen, font, 10, 50, WHITE, -1, "NEW HIGHSCORE!");
            highscore.kills[arena_idx][mode_idx] = world.kills;
            access_arena_highscore(&highscore, 0);
          }
          textprintf_ex(screen, font, 10, 100, WHITE, -1, "Press ENTER to continue...");
          while (!key[KEY_ENTER])
            chunkrest(40);
        }
        break;
      }
    }

    if (key[KEY_B])
    {
      char fname[123];
      sprintf(fname, "screenshot%d.bmp", fname_counter++);
      save_bitmap(fname, world.buf, default_palette);
      chunkrest(500);
    }
    game_loop_rest(&game_loop_clk);

    if (key[KEY_ESC])
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

  destroy_bitmap(world.buf);
  destroy_bitmap(world.spr);

  return mission;
}

void init_allegro()
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
}
