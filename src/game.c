#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "logging.h"
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
#include "keyhandling.h"
#include "sampleRegister.h"
#include "helpers.h"
#include "loadindicator.h"
#include "sprites.h"
#include "record_file.h"
#include "game_playback.h"
#include "gen_version_info.h"
#include "midi_playback.h"
#include "synth/wt_sample_loader.h"
#include "game.h"
#include "potion_logic.h"
#include "enemy_logic.h"
#include "boss_logic.h"
#include "bullet_logic.h"

void game(GlobalGameState *ggs)
{
  // For playback recording filenames
  static int fname_counter = 0;
  pr_reset_random();

  long completetime = 0;

  int *mission = &ggs->mission;
  int *game_modifiers = &ggs->game_modifiers;
  Enemy *plrautosave = &ggs->plrautosave;
  int no_player_damage = ggs->cheats & 1;

  World world;
  memset(&world, 0, sizeof(World));
  world.game_modifiers = *game_modifiers;
  world.mission = *mission;
  world.boss_fight_config = world.boss_fight_configs;
  if (!get_game_settings()->custom_resources)
  {
    world.spr = al_load_bitmap(DATADIR "sprites.png");
  }
  else
  {
    char path[256];
    sprintf(path, DATADIR "\\%s\\sprites.png", get_game_settings()->mission_pack);
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

  world.current_room = 1;

  init_world(&world);
  read_enemy_configs(&world);
  init_player(&world, plrautosave);

  long playback_next_event_time_stamp = 0;
  long key_press_mask = 0;

  world.hint.time_shows = 0;

  int *record_mode = get_playback_mode();
  if (*record_mode == RECORD_MODE_RECORD)
  {
    char record_input_filename[256];
    sprintf(record_input_filename, "recorded-mission%d-take%d.dat", *mission, ++fname_counter);
    remove(record_input_filename);

    game_playback_set_filename(record_input_filename);
    game_playback_init();
    save_game_save_data(record_input_filename, &world.plr, *mission, *game_modifiers, 0);
  }
  else if (*record_mode == RECORD_MODE_PLAYBACK)
  {
    game_playback_init();
    load_game_save_data(game_playback_get_filename(), &world.plr, mission, &world.game_modifiers, 0);
    playback_next_event_time_stamp = game_playback_get_time_stamp();
  }
  long time_stamp = 0;

  read_level(&world, *mission, 1);

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
  if (world.game_modifiers & GAMEMODIFIER_NO_GOLD)
  {
    world.plr.gold = 0;
    world.plr.ammo = 0;
  }

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

  create_sparkles(world.plr.x, world.plr.y, 30, -1, 10, &world);

  clock_t game_loop_clk = clock();

  while (restart_requested < 2)
  {
    if (world.plr.health <= 0)
    {
      // Draw well outside of screen so that the zoom in transformation would not look like ass
      rectfill(0, 0, SCREEN_W * 2, SCREEN_H * 2, BLACK);
    }
    else if (check_key(ALLEGRO_KEY_M))
    {
      show_ingame_info_screen(&world);
    }

    cleanup_bodyparts(&world);
    if (time_stamp % 6 == 0)
    {
      reset_sample_triggers();
    }
    time_stamp++;
    draw_map(&world, 0, vibrations);
    move_and_draw_body_parts(&world);
    draw_wall_shadows(&world);
    // draw_enemy_shadows(&world); -- doesn't look very good and the shadow physics are f'd up

    if (world.playcount > 0)
      world.playcount--;

    // Draw legend to same position as player
    // Legend cannot be drawn here or it would get obscured
    // by walls etc.
    int legend_x = world.plr.x;
    int legend_y = world.plr.y;
    int key_left, key_right, key_up, key_down, key_space = 0,
                                               key_x, key_z, key_a, key_s, key_d, key_f;
    if (world.plr.health > 0)
    {
      draw_enemy(&world.plr, &world);

      if (*record_mode == RECORD_MODE_PLAYBACK)
      {
        int has_more = 1;
        if (time_stamp >= playback_next_event_time_stamp)
        {
          key_press_mask = game_playback_get_key_mask();
          game_playback_next();
          playback_next_event_time_stamp = game_playback_get_time_stamp();
          has_more = playback_next_event_time_stamp != -1;
          LOG("Timestamp=%ld, keymask=0x%lx, Next timestamp: %ld\n",
              time_stamp, key_press_mask, playback_next_event_time_stamp);
        }
        if (!has_more)
        {
          world.hint.time_shows = 0;
          trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

          display_level_info(&world, *mission, get_game_settings()->mission_count, completetime);

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

      world.potion_turbo_mode = (world.game_modifiers & GAMEMODIFIER_POTION_ON_DEATH) && key_space && world.potion_duration > 0;

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

      if (check_key(ALLEGRO_KEY_R) && *record_mode != RECORD_MODE_PLAYBACK)
      {
        restart_requested = 1;
      }
      else if (restart_requested)
        restart_requested = 2;

      if (*record_mode == RECORD_MODE_RECORD)
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

          game_playback_add_key_event(time_stamp, key_press_mask);
          game_playback_next();
        }
      }
    }
    else
    {
      world.plr.move = 0;
    }
    {
      int plr_speed = 3;
      if (check_potion_effect(&world, POTION_EFFECT_FAST_PLAYER))
      {
        plr_speed++;
        if (world.potion_turbo_mode)
        {
          plr_speed += 2;
        }
      }
      for (; plr_speed >= 0; plr_speed--)
      {
        move_enemy(&world.plr, &world);
      }
    }

    change_room_if_at_exit_point(&world, *mission);

    if (get_tile_at(&world, world.plr.x, world.plr.y)->is_exit_level && world.plr.health > 0)
    {
      world.hint.time_shows = 0;
      trigger_sample_with_params(SAMPLE_WARP, 255, 127, 500);

      display_level_info(&world, *mission, get_game_settings()->mission_count, completetime);

      if (*record_mode != RECORD_MODE_PLAYBACK)
        wait_key_press(ALLEGRO_KEY_ENTER);

      if (world.final_level)
      {
        *mission = -1;
        break;
      }

      *mission++;
      *plrautosave = world.plr;
      break;
    }

    int potion_effect_divider = world.potion_turbo_mode ? 2 : 1;

    if (world.plr.health > 0 && check_potion_effect(&world, POTION_EFFECT_HEALING))
    {
      if (world.potion_healing_counter <= 0)
      {
        if (world.plr.health < 6)
          world.plr.health++;
        world.potion_healing_counter = 25;
      }
      else
      {
        world.potion_healing_counter -= potion_effect_divider;
      }
    }

    if (world.plr.health > 0 && check_potion_effect(&world, POTION_EFFECT_SHIELD_OF_FIRE))
    {
      if (world.potion_shield_counter <= 0)
      {
        create_cluster_explosion(&world, world.plr.x, world.plr.y, 4, 1, &world.plr);
        world.potion_shield_counter = 15;
      }
      else
      {
        world.potion_shield_counter -= potion_effect_divider;
      }
    }

    enemy_logic(&world);

    if (world.plr.health > 0)
    {
      bullet_logic(&world, ggs);
      potion_logic(&world);
    }

    draw_map(&world, 1, 0);

    vibrations = progress_and_draw_explosions(&world);
    if (get_game_settings()->vibration_mode != 1)
    {
      if (get_game_settings()->vibration_mode == 0)
      {
        vibrations = 0;
      }
      else
      {
        vibrations /= get_game_settings()->vibration_mode;
      }
    }

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
          if (*record_mode == RECORD_MODE_NONE && highscore_kills < world.kills)
          {
            al_draw_textf(get_font(), WHITE, offx + 10, offy + 30, ALLEGRO_ALIGN_LEFT, "Previous highscore: %d", highscore_kills);
            al_draw_textf(get_font(), WHITE, offx + 10, offy + 50, ALLEGRO_ALIGN_LEFT, "NEW HIGHSCORE!");
            highscore.kills[arena_idx][mode_idx] = world.kills;
            highscore.dirty[arena_idx][mode_idx] = 1;
            if (!no_player_damage)
            {
              access_arena_highscore(&highscore, 0);
            }
            else
            {
              LOG("Not saving highscore, no damage mode active");
            }
          }
          else
          {
            al_draw_textf(get_font(), WHITE, offx + 10, offy + 30, ALLEGRO_ALIGN_LEFT, "Highscore: %d", highscore_kills);
          }
          al_draw_textf(get_font(), WHITE, offx + 10, offy + 100, ALLEGRO_ALIGN_LEFT, "ENTER = replay, ESC = go back to menu");
          al_flip_display();
          int wait_keys[] = {ALLEGRO_KEY_ENTER, ALLEGRO_KEY_ESCAPE};
          int key = wait_key_presses(wait_keys, 2);
          if (key == ALLEGRO_KEY_ESCAPE)
          {
            menu(0, ggs);
          }
        }
        break;
      }
    }

    game_loop_rest(&game_loop_clk);

    if (check_key(ALLEGRO_KEY_ESCAPE))
    {
      if (*record_mode == RECORD_MODE_PLAYBACK)
      {
        break;
      }
      world.hint.time_shows = 0;
      int switch_level = menu(1, ggs);
      if (switch_level)
      {
        break;
      }
    }
  }

  if (*record_mode == RECORD_MODE_RECORD)
  {
    // This needs to be added so that the game does not end abruptly
    // if there are no key events near the recording end
    game_playback_add_key_event(time_stamp, 0);
    game_playback_next();
    game_playback_add_end_event();
  }

  al_destroy_bitmap(world.spr);

  {
    ALLEGRO_TRANSFORM transform;
    al_identity_transform(&transform);
    al_use_transform(&transform);
  }
}