#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
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

extern int music_on;
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
    printf("Record mode active.\n");
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
      printf("Valid input file required (--file=<filename>)!!\n");
      return 0;
    }
    printf("Playback mode active.\n");
  }
  
  read_settings();
  init_allegro();
  srand((int)time(NULL));
  int mission = 1;
  int game_modifiers = 0;
  GameFont = load_font(FONT_FILENAME, default_palette, NULL);

  register_sample(SAMPLE_SELECT, ".\\dataloss\\sel.wav");
  register_sample(SAMPLE_WARP, ".\\dataloss\\warp.wav");
  register_sample(SAMPLE_BOSSTALK_1, ".\\dataloss\\bt1.wav");
  register_sample(SAMPLE_BOSSTALK_2, ".\\dataloss\\bt2.wav");
  register_sample(SAMPLE_THROW, ".\\dataloss\\throw.wav");
  register_sample(SAMPLE_SELECT_WEAPON, ".\\dataloss\\select_weapon.wav");
  register_sample(SAMPLE_HEAL, ".\\dataloss\\healing.wav");
  register_sample(SAMPLE_PROTECTION, ".\\dataloss\\rune_of_protection.wav");
  register_sample(SAMPLE_TURRET, ".\\dataloss\\turret.wav");
  register_sample(SAMPLE_BLAST, ".\\dataloss\\blast.wav");

  for (int i = 0; i < 6; i++)
  {
    char loadsamplename[100];
    sprintf(loadsamplename, ".\\dataloss\\ex%d.wav", i + 1);
    register_sample(SAMPLE_EXPLOSION(i), loadsamplename);
    sprintf(loadsamplename, ".\\dataloss\\die%d.wav", i + 1);
    register_sample(SAMPLE_DEATH(i), loadsamplename);
  }

  next_track();
  menu(0, &plrautosave, &mission, &game_modifiers);

  while (mission != 0)
  {
    mission = game(mission, &game_modifiers);
  }
  destroy_registered_samples();
  if (record_input_file) fclose(record_input_file);
  
  free(game_settings.missions);
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


void enemy_logic(World *world, Coordinates *boss_waypoint, int boss_want_to_shoot)
{  // Viholliset
    for (int x = 0; x < ENEMYCOUNT; x++)
    {
      if (world->enm[x].roomid != world->current_room || world->enm[x].id == NO_OWNER)
        continue;

      if (world->plr.health > 0)
      {

        EnemyType enm_type = world->enm[x].type;
        if (enm_type != TURRET) // not a turret
        {
          Coordinates aim_at = {world->plr.x, world->plr.y};
          int aim_window = 2 + (enm_type == ALIEN_TURRET || enm_type == ARCH_MAGE ? 5 : 0);
          int reacts_to_player = sees_each_other(world->enm + x, &world->plr, world);

          if (reacts_to_player || (enm_type == ARCH_MAGE && boss_waypoint->x >= 0))
          {
            world->enm[x].move = 1;
            if (enm_type != ARCH_MAGE || (boss_want_to_shoot && reacts_to_player))
            {
              if (enm_type == ARCH_MAGE)
              {
                set_directions(&world->enm[x], &aim_at, aim_window);
              }
              int play_sample = shoot(world->enm + x, world);
              if (play_sample)
              {
                trigger_sample(SAMPLE_THROW, 255);
              }
            }

            world->enm[x].dx = world->enm[x].dy = 0;
            if (enm_type == ARCH_MAGE && boss_waypoint->x >= 0)
            {
              aim_at.x = boss_waypoint->x * TILESIZE + HALFTILESIZE;
              aim_at.y = boss_waypoint->y * TILESIZE + HALFTILESIZE;
              aim_window = 0;
              if (world->enm[x].x / TILESIZE == (int)boss_waypoint->x && world->enm[x].y / TILESIZE == (int)boss_waypoint->y)
              {
                printf("Waypoint reached\n");
                boss_waypoint->x = boss_waypoint->y = -1;
                world->boss_fight_config.state.waypoint_reached = 1;
              }
            }
            set_directions(&world->enm[x], &aim_at, aim_window);
            if (world->enm[x].dx == 0 && world->enm[x].dy == 0)
            {
              world->enm[x].dx = 1 - 2 * pr_get_random() % 2;
              world->enm[x].dy = 1 - 2 * pr_get_random() % 2;
            }
          }
          else
          {
            if (pr_get_random() % 30 == 0 )
            {
              world->enm[x].move = pr_get_random() % 2;
              world->enm[x].move = pr_get_random() % 2;
              world->enm[x].dx = 1 - (pr_get_random() % 3);
              world->enm[x].dy = 1 - (pr_get_random() % 3);
            }
          }
          if (enm_type == ARCH_MAGE)
          {
            for (int m = 0; m < world->boss_fight_config.speed; m++)
              move_enemy(world->enm + x, world);
          }
          else if (enm_type != ALIEN_TURRET)
          {
            if (enm_type == IMP || enm_type == ALIEN)
            {
              move_enemy(world->enm + x, world);
            }
            move_enemy(world->enm + x, world);
          }
          else if (world->enm[x].reload > 0)
          {
            world->enm[x].reload--;
          }
          if (enm_type != ALIEN || rand() % 32) // Alienit (muttei turretit) vilkkuvat
          {
            draw_enemy(world->enm + x, world);
          }
          if (enm_type == ARCH_MAGE)
          {
            // health bar for the boss
            for (int i = 0; i < 6; i++)
            {
              if (world->enm[x].health >= (world->boss_fight_config.health * (i + 1) / 6))
                masked_blit(world->spr, world->buf, 60, 0, world->enm[x].x - 23, world->enm[x].y - 18 + 4 * i, 7, 6);
            }
          }
        }
        else // turret
        {
          if (world->enm[x].move > 0)
          {
            for (int i = 0; i < world->enm[x].move; i++)
              move_enemy(world->enm + x, world);
            world->enm[x].move--;
          }
          else
          {
            float circular = cos((float)(world->enm[x].ammo / 4 % 32) * M_PI / 16) * 4;
            world->enm[x].dx = (int)circular; //(circular > 0.1) - 1 * (circular < -0.1);
            circular = sin((float)(world->enm[x].ammo / 4 % 32) * M_PI / 16) * 4;
            world->enm[x].dy = (int)circular; //(circular > 0.1) - 1 * (circular < -0.1);

            int play_sample = shoot(world->enm + x, world);
            if (play_sample)
            {
              // play_sample(s_throw, 255, 127, 800 + rand() % 400, 0);
              trigger_sample(SAMPLE_THROW, 255);
            }

            world->enm[x].reload--;
          }
          draw_enemy(world->enm + x, world);
          if (world->enm[x].ammo == 0)
          {
            //if (playcount == 0)
                trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
//              play_sample(s_expl[rand() % 6], 200, 127, 800 + rand() % 400, 0);
           // playcount = PLAYDELAY;
            create_shade_around_hit_point(world->enm[x].x, world->enm[x].y, 9, world);
            create_explosion(world->enm[x].x, world->enm[x].y, world);
            create_explosion(world->enm[x].x, world->enm[x].y, world);
            create_explosion(world->enm[x].x, world->enm[x].y, world);
            world->enm[x].ammo = -1;
            world->enm[x].shots = 1;
            world->enm[x].id = NO_OWNER;
          }
        }
      }
      else
        draw_enemy(world->enm + x, world);
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

void boss_logic(World *world, int *boss_want_to_shoot, Coordinates *boss_waypoint, int boss_died)
{
  Enemy *boss = NULL;
  if (world->boss_fight)
  {
    for (int x = 0; x < ENEMYCOUNT; x++)
    {
      if (world->enm[x].type == ARCH_MAGE)
      {
        boss = &world->enm[x];
        break;
      }
    }
  }
  int in_same_room = boss != NULL && boss->roomid == world->current_room;
  if (in_same_room || boss_died)
  {
    world->boss_fight_config.state.health = boss_died ? 0 : boss->health;
    bossfight_process_event_triggers(&world->boss_fight_config);
    for (int x = 0; x < BFCONF_MAX_EVENTS; x++)
    {
      if (!world->boss_fight_config.state.triggers[x]) continue;
      
      BossFightEventConfig *event = &world->boss_fight_config.events[x];
      
      printf("Trigger %c\n", event->event_type);
      switch (event->event_type)
      {
        case BFCONF_EVENT_TYPE_SPAWN:
        {
          BossFightSpawnPointConfig *spawn_point =  &event->spawn_point;
          int random_num = pr_get_random() % 100;
          for (int spawn_type = 0; spawn_type < 5; spawn_type++)
          {
            if (random_num >= spawn_point->probability_thresholds[spawn_type][0]
              && random_num < spawn_point->probability_thresholds[spawn_type][1])
            {
              spawn_enemy(spawn_point->x, spawn_point->y, spawn_type + 200, world->current_room, world);
              break; 
            }
          }
        }
        break;
        case BFCONF_EVENT_TYPE_ALLOW_FIRING:
          *boss_want_to_shoot = 1;
        break;
        case BFCONF_EVENT_TYPE_DISALLOW_FIRING:
          *boss_want_to_shoot = 0;
        break;
        case BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE:
          if (boss)
            create_cluster_explosion(world, boss->x, boss->y, event->parameters[0], event->parameters[1], boss->id);
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
            world->map[event->parameters[0]][event->parameters[1]] = create_tile(TILE_SYM_FLOOR);
            trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
            for (int y = 0; y < 3; y++)
              create_explosion(event->parameters[0] * TILESIZE + TILESIZE / 2, event->parameters[1] * TILESIZE + TILESIZE / 2, world);
          }
        }
        break;
        case BFCONF_EVENT_TYPE_SET_WAYPOINT:
          boss_waypoint->x = event->parameters[0];
          boss_waypoint->y = event->parameters[1];
          world->boss_fight_config.state.waypoint = event->parameters[2];
          world->boss_fight_config.state.waypoint_reached = 0;
        break;
        case BFCONF_EVENT_TYPE_CLEAR_WAYPOINT:
          boss_waypoint->x = -1;
          boss_waypoint->y = -1;
          world->boss_fight_config.state.waypoint = 0;
        break;
      }
    }
  }
}

int game(int mission, int *game_modifiers)
{
  pr_reset_random();

  long completetime = 0;

  World world;
  world.game_modifiers = *game_modifiers;
  memset(&world.boss_fight_config, 0, sizeof(BossFightConfig));
  world.buf = create_bitmap(480, 360);
  world.spr = load_bitmap(".\\dataloss\\sprites.bmp", default_palette);
  world.explos_spr = load_bitmap(".\\dataloss\\explosions.bmp", default_palette);
  BITMAP *bmp_levclear = load_bitmap(".\\dataloss\\levelclear.bmp", default_palette);

  char c;
  int vibrations = 0;
  int x, y, i, playcount = 0, additt_anim = 0;

  char hint_text[256];
  int hint_time_shows = 0, hint_x, hint_y, hint_dim;
  int fly_in_text_x = world.buf->w;
  int boss_fight_frame_count = 0;
  int boss_want_to_shoot = 0;
  char fly_in_text[64];
  strcpy(fly_in_text, game_settings.missions[mission - 1].name);

  world.current_room = 1;

  init_world(&world);
  init_player(&world, &plrautosave);
  
  FILE *f_key_presses = NULL;
  ContinuousData key_presses[128];
  int key_press_buffer_sz = 128;
  int key_press_buffer_idx = 0;
  long key_press_mask = 0;
  
  Coordinates boss_waypoint = {-1, -1};

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

  read_level(&world, game_settings.missions[mission - 1].filename, 1);
  if (world.boss_fight)
  {
    trigger_sample_with_params(SAMPLE_BOSSTALK_1, 255, 127, 1000);
  }
  
  int difficulty = (world.game_modifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : 0;
  
  if (world.plr.gold > (difficulty == DIFFICULTY_BRUTAL ? 0 : 5))
  {
    int excess_gold = world.plr.gold - (difficulty == DIFFICULTY_BRUTAL ? 0 : 5);
    world.plr.health += excess_gold * (difficulty == DIFFICULTY_BRUTAL ? 2 : 3);
    world.plr.health = world.plr.health > 6 ? 6 : world.plr.health;
    world.plr.gold = 5;
  }
  
  if (world.plr.health < 3)
    world.plr.health = 3;
  if (world.plr.ammo < 10)
    world.plr.ammo = 10;
  
  int cluster_strength = 16;
  
  if ((world.game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD) != 0)
  {
    cluster_strength = 5;
    world.plr.gold = 40;
  }
  else if (difficulty == DIFFICULTY_BRUTAL)
    world.plr.gold = 0;

  if (world.boss_fight && world.boss_fight_config.player_initial_gold >= 0)
  {
    world.plr.gold = world.boss_fight_config.player_initial_gold;
  }
    
  int plr_rune_of_protection_active = 0;

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
  while (restart_requested < 2)
  {
    if (time_stamp % 3 == 0) reset_sample_triggers();
    time_stamp++;
    draw_map(&world, -1 * vibrations); // shadows
    move_and_draw_body_parts(&world);

    if (playcount > 0)
      playcount--;

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
           if (!has_more) break;
           
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
        int activated_powerup = handle_power_up_keys(&world, key_a, key_s, key_d, key_f, &gold_hint_amount, &plr_rune_of_protection_active);
        play_sample = activated_powerup || play_sample;
        if (gold_hint_amount)
        {
          show_gold_hint(&world, hint_text, &hint_x, &hint_y, &hint_dim, &hint_time_shows, gold_hint_amount);
        }
        
                
        if (key[KEY_R])
        {
          restart_requested = 1;
        }
        else if (restart_requested) restart_requested = 2;
        
        if (record_mode == RECORD_MODE_RECORD)
        {
            long new_key_press_mask = (key[KEY_LEFT] ? 1 : 0) 
                 | (key[KEY_RIGHT] ? 2 : 0) 
                 | (key[KEY_UP] ? 4 : 0) 
                 | (key[KEY_DOWN] ? 8 : 0) 
                 | (key[KEY_SPACE] ? 16 : 0)
                 | (key[KEY_X] ? 32 : 0)
                 | (key[KEY_Z] ? 64 : 0)
                 | (key[KEY_A] ? 128 : 0)
                 | (key[KEY_S] ? 256 : 0)
                 | (key[KEY_D] ? 512 : 0)
                 | (key[KEY_F] ? 1024 : 0);
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
      hint_time_shows = 0;
      trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

      display_level_info(&world, mission, game_settings.mission_count, bmp_levclear, font);

      while (!key[KEY_ENTER])
      {
        chunkrest(15);
      }
      chunkrest(250);
      mission++;
      plrautosave = world.plr;
      break;
    }
    
    enemy_logic(&world, &boss_waypoint, boss_want_to_shoot);

    // ammukset

    if (world.plr.health > 0)
      for (x = 0; x < BULLETCOUNT; x++)
      {

        if (world.bullets[x].owner_id == NO_OWNER)
          continue;
        int z;
        double bullet_orig_x = world.bullets[x].x;
        double bullet_orig_y = world.bullets[x].y;
        for (z = 0; z < 12; z++)
        {
          move_bullet(world.bullets + x, &world);
          if (world.bullets[x].owner_id == NO_OWNER)
          {
            if (playcount == 0)
               trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
            playcount = PLAYDELAY;
            create_explosion(world.bullets[x].x, world.bullets[x].y, &world);
            if (world.bullets[x].bullet_type == BULLET_TYPE_CLUSTER)
            {
              world.bullets[x].x = ((int)(bullet_orig_x / TILESIZE)) * TILESIZE + HALFTILESIZE;
              world.bullets[x].y = ((int)(bullet_orig_y / TILESIZE)) * TILESIZE + HALFTILESIZE;
              while (check_flags_at(&world, (int)world.bullets[x].x, (int)world.bullets[x].y, TILE_IS_WALL))
              {
                  world.bullets[x].x -= 5 * world.bullets[x].dx;
                  world.bullets[x].y -= 5 * world.bullets[x].dy;
              }
              create_cluster_explosion(&world, world.bullets[x].x, world.bullets[x].y, 16, cluster_strength, PLAYER_ID);
            }
            
            break;
          }
          if (world.bullets[x].owner_id < 9000 && bullet_hit(&world.plr, world.bullets + x)) // Player gets hit
          {
            if (plr_rune_of_protection_active == 1)
            {
                world.plr.health++;
                if (world.plr.health < 0) world.plr.health = 1;
                world.plr.id = PLAYER_ID;
                plr_rune_of_protection_active = -50;
                create_cluster_explosion(&world, world.plr.x, world.plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, PLAYER_ID);
                if ((world.game_modifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0)
                {
                  create_cluster_explosion(&world, world.plr.x, world.plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, PLAYER_ID);
                }
            }
            if (playcount == 0)
                trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
            playcount = PLAYDELAY;
            create_shade_around_hit_point(world.plr.x, world.plr.y, 9, &world);
            create_explosion(world.plr.x, world.plr.y, &world);
            if (world.plr.health <= 0)
            {
              create_explosion(world.plr.x - 20, world.plr.y - 20, &world);
              create_explosion(world.plr.x + 20, world.plr.y + 20, &world);
              create_explosion(world.plr.x - 20, world.plr.y + 20, &world);
              create_explosion(world.plr.x + 20, world.plr.y - 20, &world);

              chunkrest(1); // The death sample won't play else
              trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (world.enm[y].x - 240) / 8, 900 + rand() % 200);
              world.plr.reload = 100;
              break;
            }
          }
          int deathsample_plays = 0;
          if (world.bullets[x].hurts_monsters)
            for (y = 0; y < ENEMYCOUNT; y++)
            {
              if (world.enm[y].id == NO_OWNER || world.enm[y].id >= 9000 || world.enm[y].roomid != world.current_room)
                continue;

              if (bullet_hit(world.enm + y, world.bullets + x))
              {
                create_shade_around_hit_point(world.enm[y].x, world.enm[y].y, 9, &world);
                create_explosion(world.enm[y].x, world.enm[y].y, &world);
                create_explosion(world.enm[y].x, world.enm[y].y, &world);
                create_explosion(world.enm[y].x, world.enm[y].y, &world);
                if (!deathsample_plays)
                  if (playcount == 0)
                    trigger_sample(SAMPLE_EXPLOSION(rand() % 6), 200);
                    
                if (world.bullets[x].bullet_type == BULLET_TYPE_CLUSTER)
                {
                    create_cluster_explosion(&world, world.bullets[x].x, world.bullets[x].y, 16, cluster_strength, PLAYER_ID);
                }
                    
                playcount = PLAYDELAY;
                if (world.enm[y].id == NO_OWNER) // enemy was killed (bullet_hit has side effects)
                {
                  set_tile_flag(&world, world.enm[y].x, world.enm[y].y, TILE_IS_BLOOD_STAINED);
                  world.plr.gold += world.enm[y].gold;

                  if (world.enm[y].gold > 0)
                  {
                    sprintf(hint_text, "+ %d", world.enm[y].gold);
                    hint_x = world.enm[y].x - 15;
                    hint_y = world.enm[y].y - 15;
                    hint_dim = 6;
                    hint_time_shows = 40;
                  }
                  if((world.game_modifiers & GAMEMODIFIER_MULTIPLIED_GOLD) == 0)
                  {
                      if (world.plr.health < 3 && world.plr.health > 0)
                        world.plr.health++;
                      world.plr.ammo += 7;
                      if (world.plr.ammo > 15)
                        world.plr.ammo = 15;
                  }

                  if (world.enm[y].type == ARCH_MAGE) // Archmage dies
                  {
                    printf("boss die logic\n");
                    boss_logic(&world, &boss_want_to_shoot, &boss_waypoint, 1);
                    chunkrest(1);
                    trigger_sample_with_params(SAMPLE_BOSSTALK_2, 255, 127 + (world.enm[y].x - 240) / 8, 1000);
                  }
                  else
                  {
                    chunkrest(1); // The death sample won't play else
                    if (!deathsample_plays)
                     trigger_sample_with_params(SAMPLE_DEATH(rand() % 6), 255, 127 + (world.enm[y].x - 240) / 8, 900 + rand() % 200);
                  }
                  deathsample_plays = 1;
                  spawn_body_parts(&world.enm[y]);
                }
                break;
              }
            }
        }
        if (world.bullets[x].bullet_type == BULLET_TYPE_NORMAL)
        {
            int bullet_sprite = ((int)(world.bullets[x].x + world.bullets[x].y) / 30) % 4;
            masked_blit(world.spr, world.buf, 140 + bullet_sprite * 10, 140, world.bullets[x].x - 5, world.bullets[x].y - 5, 10, 10);
        } else if (world.bullets[x].bullet_type == BULLET_TYPE_CLUSTER)
        {
            masked_blit(world.spr, world.buf, 140, 150, world.bullets[x].x - 6, world.bullets[x].y - 6, 13, 12);
        }
      }

    draw_map(&world, mission % 3 + 1);

    draw_player_legend(&world);
    
    completetime++;

    // Draw hint

    if (hint_time_shows > 0)
    {
      hint_time_shows--;
      int hint_col = hint_time_shows * hint_dim;
      textprintf_ex(world.buf, font, hint_x, hint_y, GRAY(hint_col), -1, hint_text);
    }
    if (world.boss_fight && ++boss_fight_frame_count >= 3)
    {
      boss_fight_frame_count = 0;
      boss_logic(&world, &boss_want_to_shoot, &boss_waypoint, 0);
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
    
    
    if (plr_rune_of_protection_active)
    {
        if (plr_rune_of_protection_active < 0)
        {
           plr_rune_of_protection_active++;
           masked_blit(world.spr, world.buf, 140, 165, 
                   world.plr.x + plr_rune_of_protection_active * sin(completetime * 0.15) - 7, 
                   world.plr.y + plr_rune_of_protection_active * cos(completetime * 0.15) - 7, 
                   13, 13);
        }
        else masked_blit(world.spr, world.buf, 140, 165, 
                               world.plr.x - TILESIZE * sin(completetime * 0.15) - 7, 
                               world.plr.y - TILESIZE * cos(completetime * 0.15) - 7, 
                               13, 13);
    }
    
    if (fly_in_text_x > -400)
    {
      textprintf_ex(world.buf, font, fly_in_text_x, 170, GRAY(255), -1, fly_in_text);
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
      chunkrest(20);
      if (world.plr.reload <= 0)
        break;
    }

    //play_mp3();
    if (key[KEY_B])
    {
      char fname[123];
      sprintf(fname, "screenshot%d.bmp", fname_counter++);
      save_bitmap(fname, world.buf, default_palette);
      chunkrest(500);
    }
    chunkrest(15);

    if (key[KEY_ESC])
    {

      hint_time_shows = 0;
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
  destroy_bitmap(world.explos_spr);
  destroy_bitmap(bmp_levclear);

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
   printf("starting in fullscreen\n");
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
  printf("allegro inited\n");
}
