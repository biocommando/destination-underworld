#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "game.h"
#include "logging.h"
#include "duconstants.h"
#include "world.h"
#include "menu.h"
#include "settings.h"
#include "sampleRegister.h"
#include "loadindicator.h"
#include "sprites.h"
#include "record_file.h"
#include "game_playback.h"
#include "gen_version_info.h"
#include "midi_playback.h"
#include "synth/wt_sample_loader.h"

#ifdef ENABLE_LOGGING
int logging_enabled = 0;
#endif

int main(int argc, char **argv)
{
  printf("Destination Underworld " DU_VERSION "\n");
  printf("Command line arguments attempted to read:\n");

  char read_arg[256] = "";
#ifdef ENABLE_LOGGING
  logging_enabled = read_cmd_line_arg_int("logging", argv, argc);
#endif
  read_cmd_line_arg_str("player-damage", argv, argc, read_arg);
  int player_damage_off = 0;
  if (!strcmp(read_arg, "off"))
  {
    player_damage_off = 1;
    LOG("Player damage off\n");
  }
  read_arg[0] = 0;

  read_cmd_line_arg_str("record-mode", argv, argc, read_arg);
  int *record_mode = get_playback_mode();
  int record_playback_no_user_interaction = 0;
  if (!strcmp(read_arg, "record"))
  {
    *record_mode = RECORD_MODE_RECORD;
    LOG("Record mode active.\n");
  }
  else if (!strcmp(read_arg, "play"))
  {
    *record_mode = RECORD_MODE_PLAYBACK;
    char fname[256];
    FILE *record_input_file = NULL;
    if (read_cmd_line_arg_str("file", argv, argc, fname))
    {
      record_input_file = fopen(fname, "r");
      game_playback_set_filename(fname);
    }
    if (record_input_file == NULL)
    {
      LOG("Valid input file required (--file=<filename>)!!\n");
      return 0;
    }
    fclose(record_input_file);
    record_playback_no_user_interaction = read_cmd_line_arg_int("start-without-user-interaction", argv, argc);
    LOG("Playback mode active.\n");
  }
  else if (read_arg[0])
  {
    printf("Record mode must be either 'play' or 'record'.\n");
    return 1;
  }

  read_settings(argv, argc);
  wt_sample_read_all(DATADIR);
  init_allegro();
  progress_load_state("Loading game...", 1);
  srand((int)time(NULL));
  int game_modifiers = read_cmd_line_arg_int("default-game-mode", argv, argc);

  progress_load_state("Loading samples...", 1);
  register_sample(SAMPLE_WARP, "warp", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_BOSSTALK_1, "boss_level_start", SAMPLE_PRIORITY(HIGH, 2), 0);
  register_sample(SAMPLE_BOSSTALK_2, "boss_level_boss_dies", SAMPLE_PRIORITY(HIGH, 2), 0);
  register_sample(SAMPLE_THROW, "throw", SAMPLE_PRIORITY(LOW, 0), 0);
  register_sample(SAMPLE_SELECT_WEAPON, "select_weapon", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_HEAL, "powerup_healing", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_PROTECTION, "powerup_protection", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_TURRET, "powerup_turret", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_BLAST, "powerup_megablast", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_SPAWN, "spawn", SAMPLE_PRIORITY(HIGH, 0), 0);
  register_sample(SAMPLE_POTION(POTION_ID_SHIELD), "potion_shield", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_POTION(POTION_ID_MINOR_SHIELD), "potion_minor_shield", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_POTION(POTION_ID_STOP), "potion_stop", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_POTION(POTION_ID_FAST), "potion_fast", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_POTION(POTION_ID_BOOST), "potion_boost", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_POTION(POTION_ID_HEAL), "potion_heal", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_POTION(POTION_ID_INSTANT_HEAL), "potion_instant_heal", SAMPLE_PRIORITY(HIGH, 1), 0);

  for (int i = 0; i < 6; i++)
  {
    char loadsamplename[100];
    sprintf(loadsamplename, "explosion_%d", i + 1);
    register_sample(SAMPLE_EXPLOSION(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0), 1);
    sprintf(loadsamplename, "death_%d", i + 1);
    register_sample(SAMPLE_DEATH(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0), 2);
  }

  for (int i = 0; i < 3; i++)
  {
    char loadsamplename[100];
    sprintf(loadsamplename, "blood_splash_%d", i + 1);
    register_sample(SAMPLE_SPLASH(i), loadsamplename, SAMPLE_PRIORITY(NORMAL, 0), 3);
  }

  register_sample(SAMPLE_MENU_CHANGE, "menu_change", SAMPLE_PRIORITY(HIGH, 1), 0);
  register_sample(SAMPLE_MENU_SELECT, "menu_select", SAMPLE_PRIORITY(HIGH, 1), 0);

  progress_load_state("Loading sprites...", 1);
  {
    char path[256];
    get_data_filename(path, "sprites.dat");
    read_sprites_from_file(path, SPRITE_ID_MIN, SPRITE_ID_MAX);
  }

  progress_load_state("Loading menu...", 1);
  randomize_midi_playlist();
  next_midi_track(-1);

  GlobalGameState ggs;
  memset(&ggs, 0, sizeof(ggs));
  if (player_damage_off)
    ggs.cheats |= 1;
  ggs.mission = 1;
  ggs.no_player_interaction = record_playback_no_user_interaction;
  ggs.setup_screenshot_buffer = read_cmd_line_arg_int("screenshot-buffer", argv, argc);
  while (ggs.mission != 0)
  {
    if (*record_mode != RECORD_MODE_PLAYBACK || !ggs.no_player_interaction)
      menu(0, &ggs);
    while (ggs.mission > 0)
    {
      game(&ggs);
      if (*record_mode == RECORD_MODE_PLAYBACK)
      {
        ggs.mission = 0;
        break;
      }
    }
  }
  progress_load_state("Exiting game...", 0);
  destroy_registered_samples();

  wt_sample_free();
  record_file_flush();
  destroy_allegro();
  return 0;
}