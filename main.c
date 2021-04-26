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
//#include "scripting.h"
#include "bossfightconf.h"
#include "predictableRandom.h"
#include "settings.h"
#include "continuousData.h"
#include "keyhandling.h"
#include "sampleRegister.h"
#include "helpers.h"

extern int musicOn;
extern MP3FILE *mp3;

void initAllegro();
int game(int mission, int *gameModifiers);
void readSettings();

SAMPLE *s_throw;

//extern struct gamedata UniqueGameData;

GameSettings gameSettings;

Enemy plrautosave;

FONT *GameFont;
int fname_counter = 0;

//int difficulty = DIFFICULTY_BRUTAL;

int recordMode = RECORD_MODE_NONE;
FILE *recordInputFile = NULL;

int main(int argc, char **argv)
{
  recordMode = readCmdLineArgInt("record-mode", argv, argc);
  if (recordMode == RECORD_MODE_RECORD)
  {
    printf("Record mode active.\n");
  }
  if (recordMode == RECORD_MODE_PLAYBACK)
  {
    char fname[256];
    if (readCmdLineArgStr("file", argv, argc, fname))
    {
        recordInputFile = fopen(fname, "r");
    }
    if (recordInputFile == NULL) 
    {
      printf("Valid input file required (--file=<filename>)!!\n");
      return 0;
    }
    printf("Playback mode active.\n");
  }
  
  readSettings();
  initAllegro();
  srand((int)time(NULL));
  int mission = 1;
  int gameModifiers = 0;
  GameFont = load_font(FONT_FILENAME, default_palette, NULL);

  registerSample(SAMPLE_SELECT, ".\\dataloss\\sel.wav");
  registerSample(SAMPLE_WARP, ".\\dataloss\\warp.wav");
  registerSample(SAMPLE_BOSSTALK_1, ".\\dataloss\\bt1.wav");
  registerSample(SAMPLE_BOSSTALK_2, ".\\dataloss\\bt2.wav");
  registerSample(SAMPLE_THROW, ".\\dataloss\\throw.wav");
  registerSample(SAMPLE_SELECT_WEAPON, ".\\dataloss\\select_weapon.wav");
  registerSample(SAMPLE_HEAL, ".\\dataloss\\healing.wav");
  registerSample(SAMPLE_PROTECTION, ".\\dataloss\\rune_of_protection.wav");
  registerSample(SAMPLE_TURRET, ".\\dataloss\\turret.wav");
  registerSample(SAMPLE_BLAST, ".\\dataloss\\blast.wav");

/*  SAMPLE *s_expl[6], *s_die[6], *s_sel = load_sample(".\\dataloss\\sel.wav"), *s_warp = load_sample(".\\dataloss\\warp.wav");
  SAMPLE *s_bosstalk1 = load_sample(".\\dataloss\\bt1.wav"), *s_bosstalk2 = load_sample(".\\dataloss\\bt2.wav");*/

  //s_throw = load_sample(".\\dataloss\\throw.wav");
  for (int i = 0; i < 6; i++)
  {
    char loadsamplename[100];
    sprintf(loadsamplename, ".\\dataloss\\ex%d.wav", i + 1);
    registerSample(SAMPLE_EXPLOSION(i), loadsamplename);
    sprintf(loadsamplename, ".\\dataloss\\die%d.wav", i + 1);
    registerSample(SAMPLE_DEATH(i), loadsamplename);
  }

  nextTrack();
  menu(0, &plrautosave, &mission, &gameModifiers);

  while (mission != 0)
  {
    mission = game(mission, &gameModifiers);
  }
  destroyRegisteredSamples();
  if (recordInputFile) fclose(recordInputFile);
  
  free(gameSettings.missions);
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


void enemyLogic(World *world, Coordinates *boss_waypoint, int bossWantToShoot)
{  // Viholliset
    for (int x = 0; x < ENEMYCOUNT; x++)
    {
      if (world->enm[x].roomid != world->currentRoom || world->enm[x].id == NO_OWNER)
        continue;

      if (world->plr.health > 0)
      {

        EnemyType enmType = world->enm[x].type;
        if (enmType != TURRET) // not a turret
        {
          Coordinates aim_at = {world->plr.x, world->plr.y};
          int aim_window = 2 + (enmType == ALIEN_TURRET || enmType == ARCH_MAGE ? 5 : 0);
          //int reactsToPlayer = (enmType == ARCH_MAGE && (bossMovementStrategy == 0 || bossMovementStrategy == 1)) || (enmType != ARCH_MAGE && sees_each_other(world->enm + x, &world->plr, world));
          int reactsToPlayer = sees_each_other(world->enm + x, &world->plr, world);

          if (reactsToPlayer || (enmType == ARCH_MAGE && boss_waypoint->x >= 0))
          {
            world->enm[x].move = 1;
            if (enmType != ARCH_MAGE || (bossWantToShoot && reactsToPlayer))
            {
              if (enmType == ARCH_MAGE)
              {
                set_directions(&world->enm[x], &aim_at, aim_window);
              }
              /*if (enmType == ARCH_MAGE && bossMovementStrategy == 1)
              {
                world->enm[x].dx *= -1;
                world->enm[x].dy *= -1;
              }*/
              int playSample = shoot(world->enm + x, world);
              if (playSample)
              {
                //play_sample(s_throw, 255, 127, 800 + rand() % 400, 0);
                triggerSample(SAMPLE_THROW, 255);
              }
            }

            world->enm[x].dx = world->enm[x].dy = 0;
            if (enmType == ARCH_MAGE && boss_waypoint->x >= 0)
            {
              aim_at.x = boss_waypoint->x * TILESIZE + HALFTILESIZE;
              aim_at.y = boss_waypoint->y * TILESIZE + HALFTILESIZE;
              aim_window = 0;
              if (world->enm[x].x / TILESIZE == (int)boss_waypoint->x && world->enm[x].y / TILESIZE == (int)boss_waypoint->y)
              {
                printf("Waypoint reached\n");
                boss_waypoint->x = boss_waypoint->y = -1;
                world->bossFightConfig.state.waypoint_reached = 1;
              }
            }
            set_directions(&world->enm[x], &aim_at, aim_window);
            /*if (world->enm[x].x > aim_at.x + aim_window)
              world->enm[x].dx = -1;
            if (world->enm[x].x < aim_at.x - aim_window)
              world->enm[x].dx = 1;
            if (world->enm[x].y > aim_at.y + aim_window)
              world->enm[x].dy = -1;
            if (world->enm[x].y < aim_at.y - aim_window)
              world->enm[x].dy = 1;*/
            if (world->enm[x].dx == 0 && world->enm[x].dy == 0)
            {
              world->enm[x].dx = 1 - 2 * prGetRandom() % 2;
              world->enm[x].dy = 1 - 2 * prGetRandom() % 2;
            }
            /*if (enmType == ARCH_MAGE && bossMovementStrategy == 1)
            {
              world->enm[x].dx *= -1;
              world->enm[x].dy *= -1;
            }*/
          }
          else
          {
            if (prGetRandom() % 30 == 0 )//&& (enmType != ARCH_MAGE || bossMovementStrategy != 2))
            {
              world->enm[x].move = prGetRandom() % 2;
              world->enm[x].move = prGetRandom() % 2;
              world->enm[x].dx = 1 - (prGetRandom() % 3);
              world->enm[x].dy = 1 - (prGetRandom() % 3);
            }
          }
          if (enmType == ARCH_MAGE)
          {
            for (int m = 0; m < world->bossFightConfig.speed; m++)
              move_enemy(world->enm + x, world);
          }
          else if (enmType != ALIEN_TURRET)
          {
            if (enmType == IMP || enmType == ALIEN)
            {
              move_enemy(world->enm + x, world);
            }
            move_enemy(world->enm + x, world);
          }
          else if (world->enm[x].reload > 0)
          {
            world->enm[x].reload--;
          }
          if (enmType != ALIEN || rand() % 32) // Alienit (muttei turretit) vilkkuvat
          {
            draw_enemy(world->enm + x, world);
          }
          if (enmType == ARCH_MAGE)
          {
            // health bar for the boss
            for (int i = 0; i < 6; i++)
            {
              if (world->enm[x].health >= (world->bossFightConfig.health * (i + 1) / 6))
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

            int playSample = shoot(world->enm + x, world);
            if (playSample)
            {
              // play_sample(s_throw, 255, 127, 800 + rand() % 400, 0);
              triggerSample(SAMPLE_THROW, 255);
            }

            world->enm[x].reload--;
          }
          draw_enemy(world->enm + x, world);
          if (world->enm[x].ammo == 0)
          {
            //if (playcount == 0)
                triggerSample(SAMPLE_EXPLOSION(rand() % 6), 200);
//              play_sample(s_expl[rand() % 6], 200, 127, 800 + rand() % 400, 0);
           // playcount = PLAYDELAY;
            createShadeAroundHitPoint(world->enm[x].x, world->enm[x].y, 9, world);
            createExplosion(world->enm[x].x, world->enm[x].y, world);
            createExplosion(world->enm[x].x, world->enm[x].y, world);
            createExplosion(world->enm[x].x, world->enm[x].y, world);
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

void drawStaticBackground()
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

void boss_logic(World *world, int *bossWantToShoot, Coordinates *boss_waypoint, int boss_died)
{
  Enemy *boss = NULL;
  if (world->bossFight)
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
  int inSameRoom = boss != NULL && boss->roomid == world->currentRoom;
  if (inSameRoom || boss_died)
  {
    world->bossFightConfig.state.health = boss_died ? 0 : boss->health;
    bossfight_process_event_triggers(&world->bossFightConfig);
    for (int x = 0; x < BFCONF_MAX_EVENTS; x++)
    {
      if (!world->bossFightConfig.state.triggers[x]) continue;
      
      BossFightEventConfig *event = &world->bossFightConfig.events[x];
      
      printf("Trigger %c\n", event->event_type);
      switch (event->event_type)
      {
        case BFCONF_EVENT_TYPE_SPAWN:
        {
          BossFightSpawnPointConfig *spawnPoint =  &event->spawn_point;
          int random_num = prGetRandom() % 100;
          for (int spawnType = 0; spawnType < 5; spawnType++)
          {
            if (random_num >= spawnPoint->probability_thresholds[spawnType][0]
              && random_num < spawnPoint->probability_thresholds[spawnType][1])
            {
              spawnEnemy(spawnPoint->x, spawnPoint->y, spawnType + 200, world->currentRoom, world);
              break; 
            }
          }
        }
        break;
        case BFCONF_EVENT_TYPE_ALLOW_FIRING:
          *bossWantToShoot = 1;
        break;
        case BFCONF_EVENT_TYPE_DISALLOW_FIRING:
          *bossWantToShoot = 0;
        break;
        case BFCONF_EVENT_TYPE_FIRE_IN_CIRCLE:
          if (boss)
            createClusterExplosion(world, boss->x, boss->y, event->parameters[0], event->parameters[1], boss->id);
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
            world->map[event->parameters[0]][event->parameters[1]] = createTile(TILE_SYM_FLOOR);
            triggerSample(SAMPLE_EXPLOSION(rand() % 6), 200);
            for (int y = 0; y < 3; y++)
              createExplosion(event->parameters[0] * TILESIZE + TILESIZE / 2, event->parameters[1] * TILESIZE + TILESIZE / 2, world);
          }
        }
        break;
        case BFCONF_EVENT_TYPE_SET_WAYPOINT:
          boss_waypoint->x = event->parameters[0];
          boss_waypoint->y = event->parameters[1];
          world->bossFightConfig.state.waypoint = event->parameters[2];
          world->bossFightConfig.state.waypoint_reached = 0;
        break;
        case BFCONF_EVENT_TYPE_CLEAR_WAYPOINT:
          boss_waypoint->x = -1;
          boss_waypoint->y = -1;
          world->bossFightConfig.state.waypoint = 0;
        break;
      }
    }
  }
}

int game(int mission, int *gameModifiers)
{
  prResetRandom();

  long completetime = 0;

  World world;
  world.gameModifiers = *gameModifiers;
//  world.worldScriptInited = 0;
  memset(&world.bossFightConfig, 0, sizeof(BossFightConfig));
  world.buf = create_bitmap(480, 360);
  world.spr = load_bitmap(".\\dataloss\\sprites.bmp", default_palette);
  world.explosSpr = load_bitmap(".\\dataloss\\explosions.bmp", default_palette);
  BITMAP *bmp_levclear = load_bitmap(".\\dataloss\\levelclear.bmp", default_palette);

  char c;
  int vibrations = 0;
  int x, y, i, playcount = 0, addittAnim = 0;

  char hintText[256];
  int hint_timeShows = 0, hintX, hintY, hintDim;
  int flyInTextX = world.buf->w;
  int worldScriptFrameCount = 0;
  int bossMovementStrategy = 0;
  int bossWantToShoot = 0;
  char flyInText[64];
  strcpy(flyInText, gameSettings.missions[mission - 1].name);

  world.currentRoom = 1;

  initWorld(&world);
  initPlayer(&world, &plrautosave);
  
  FILE *fKeyPresses = NULL;
  ContinuousData keyPresses[128];
  int keyPressBufferSz = 128;
  int keyPressBufferIdx = 0;
  long keyPressMask = 0;
  
  Coordinates boss_waypoint = {-1, -1};

  if (recordMode == RECORD_MODE_RECORD)
  {
     char fname[100];
     sprintf(fname, "recorded-mission%d-take%d.dat", mission, ++fname_counter);
     fKeyPresses = fopen(fname, "w");
     saveGameSaveData(fKeyPresses, &world.plr, mission, *gameModifiers);
     fprintf(fKeyPresses, "\n-- data start --\n");
  }
  else if (recordMode == RECORD_MODE_PLAYBACK)
  {
    fseek(recordInputFile, 0, SEEK_SET);
    fKeyPresses = recordInputFile;
    loadGameSaveData(fKeyPresses, &world.plr, &mission, &world.gameModifiers);
    keyPressBufferIdx = -1;
  }
  long timeStamp = 0;

  readLevel(&world, gameSettings.missions[mission - 1].filename, 1);
  if (world.bossFight)
  {
    //play_sample(s_bosstalk1, 255, 127, 1000, 0);
    triggerSampleWithParams(SAMPLE_BOSSTALK_1, 255, 127, 1000);
  }
  
  int difficulty = (world.gameModifiers & GAMEMODIFIER_BRUTAL) != 0 ? DIFFICULTY_BRUTAL : 0;
  
  if (world.plr.gold > (difficulty == DIFFICULTY_BRUTAL ? 0 : 5))
  {
    int excessGold = world.plr.gold - (difficulty == DIFFICULTY_BRUTAL ? 0 : 5);
    world.plr.health += excessGold * (difficulty == DIFFICULTY_BRUTAL ? 2 : 3);
    world.plr.health = world.plr.health > 6 ? 6 : world.plr.health;
    world.plr.gold = 5;
  }
  
  if (world.plr.health < 3)
    world.plr.health = 3;
  if (world.plr.ammo < 10)
    world.plr.ammo = 10;
  
  int cluster_strength = 16;
  
  if ((world.gameModifiers & GAMEMODIFIER_MULTIPLIED_GOLD) != 0)
  {
    cluster_strength = 5;
    world.plr.gold = 40;
    /*if (world.plr.gold < 2) world.plr.gold = 2;
    world.plr.gold *= 6;*/
  }
  else if (difficulty == DIFFICULTY_BRUTAL)
    world.plr.gold = 0;

  if (world.bossFight && world.bossFightConfig.player_initial_gold >= 0)
  {
    world.plr.gold = world.bossFightConfig.player_initial_gold;
  }
    
  int plr_rune_of_protection_active = 0;

  int plr_dir_helper_intensity = 0;
  
  int restart_requested = 0;
  
/*  int cost_heal = 1;
  int cost_protection = 2;
  int cost_turret = difficulty == DIFFICULTY_BRUTAL ? 4 : 3;
  int cost_blast = difficulty == DIFFICULTY_BRUTAL ? 8 : 6;*/
//  stretch_blit(world.buf, screen, 0, 0, 480, 360, offset, offset, screen->w, screen->h);
  int screenWidthScaled, screenHOffset, screenVOffset, screenHeightScaled;
  {
    double screenRatio = 480.0 / 360.0;
    screenWidthScaled = screen->h * screenRatio;
    screenHeightScaled = screen->h;
    if (screenWidthScaled > screen->w)
    {
      screenWidthScaled = screen->w;
      screenHeightScaled = screen->w / screenRatio;
    }
    screenHOffset = (screen->w - screenWidthScaled) / 2;
    screenVOffset = (screen->h - screenHeightScaled) / 2;
  }
  
  drawStaticBackground();
  while (restart_requested < 2)
  {
    if (timeStamp % 3 == 0) resetSampleTriggers();
    timeStamp++;
    draw_map(&world, -1 * vibrations); // shadows
    moveAndDrawBodyParts(&world);

    if (playcount > 0)
      playcount--;

    vibrations = progressAndDrawExplosions(&world);
    if (gameSettings.vibrationMode != 1)
    {
     if (gameSettings.vibrationMode == 0)
     {
      vibrations = 0;
     }
     else
     {
      vibrations /= gameSettings.vibrationMode;
     }
    }
    

    if (world.plr.health > 0)
    {
        draw_enemy(&world.plr, &world);
/*       
        world.plr.move = 0;
         int orig_dx = world.plr.dx;
        int orig_dy = world.plr.dy;*/
        
        int keyLeft, keyRight, keyUp, keyDown, keySpace, 
            keyX, keyZ, keyA, keyS, keyD, keyF;
        if (recordMode == RECORD_MODE_PLAYBACK)
        {
           int hasMore = readAndProcessContinuousData(keyPresses, 
               &keyPressBufferSz, &keyPressBufferIdx, fKeyPresses, 
               timeStamp, getKeyPresses, &keyPressMask);
           if (!hasMore) break;
           
            keyLeft = keyPressMask & 1;
            keyRight = keyPressMask & 2;
            keyUp = keyPressMask & 4;
            keyDown = keyPressMask & 8;
            keySpace = keyPressMask & 16;
            keyX = keyPressMask & 32;
            keyZ = keyPressMask & 64;
            keyA = keyPressMask & 128;
            keyS = keyPressMask & 256;
            keyD = keyPressMask & 512;
            keyF = keyPressMask & 1024;
        }
        else
        {
            keyLeft = key[KEY_LEFT];
            keyRight = key[KEY_RIGHT];
            keyUp = key[KEY_UP];
            keyDown = key[KEY_DOWN];
            keySpace = key[KEY_SPACE];
            keyX = key[KEY_X];
            keyZ = key[KEY_Z];
            keyA = key[KEY_A];
            keyS = key[KEY_S];
            keyD = key[KEY_D];
            keyF = key[KEY_F];
        }
        
        int playerDidChangeDir = handleDirectionKeys(&world, keyUp, keyDown, keyLeft, keyRight);
        if (playerDidChangeDir)
        {
           plr_dir_helper_intensity = PLR_DIR_HELPER_INITIAL_INTENSITY;
        }
          
        
        int playSample = handleShootKey(&world, keySpace);
        
        if (playSample)
        {
//            play_sample(s_throw, 255, 127, 800 + rand() % 400, 0);
              triggerSample(SAMPLE_THROW, 255);
        }
        
        /*if (key[KEY_U] && key[KEY_N] && key[KEY_LCONTROL] && world.plr.health > 0) // cheat for dev
        {
          world.plr.health = 3;
          world.plr.ammo = 15;
          world.plr.gold = 15;
        }
        if (key[KEY_U] && key[KEY_N] && key[KEY_RCONTROL] && world.plr.health > 0) // cheat for dev
        {
          world.plr.health = 30;
        }*/
        
        playSample = handleWeaponChangeKeys(&world, keyX, keyZ);
        if (playSample)
        {
//            play_sample(s_sel, 255, 127, 1000, 0);
              triggerSampleWithParams(SAMPLE_SELECT_WEAPON, 127, 127, 1000);
        }
        
        int goldHintAmount = 0;
        int activatedPowerup = handlePowerUpKeys(&world, keyA, keyS, keyD, keyF, &goldHintAmount, &plr_rune_of_protection_active);
        playSample = activatedPowerup || playSample;
        if (goldHintAmount)
        {
          show_gold_hint(&world, hintText, &hintX, &hintY, &hintDim, &hint_timeShows, goldHintAmount);
        }
        
                
        if (key[KEY_R])
        {
          restart_requested = 1;
        }
        else if (restart_requested) restart_requested = 2;
        
        if (recordMode == RECORD_MODE_RECORD)
        {
            long newKeyPressMask = (key[KEY_LEFT] ? 1 : 0) 
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
            if (newKeyPressMask != keyPressMask)
            {
              keyPressMask = newKeyPressMask;
              produceAndWriteContinuousData(keyPresses, keyPressBufferSz, &keyPressBufferIdx, fKeyPresses, timeStamp, 1, keyPressMask);
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

    changeRoomIfAtExitPoint(&world, mission);


    if (getTileAt(&world, world.plr.x, world.plr.y).flags & TILE_IS_EXIT_LEVEL && world.plr.health > 0)
    {
      hint_timeShows = 0;
//      play_sample(s_warp, 255, 127, 500, 0);
      triggerSampleWithParams(SAMPLE_WARP, 255, 127, 500);

      //NewUniqueGameId(&UniqueGameData);
      displayLevelInfo(&world, mission, gameSettings.missionCount, bmp_levclear, font);

      while (!key[KEY_ENTER])
      {
        //playMP3();
        chunkrest(15);
      }
      chunkrest(250);
      mission++;
      plrautosave = world.plr;
      break;
    }
    
    enemyLogic(&world, &boss_waypoint, bossWantToShoot);

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
               triggerSample(SAMPLE_EXPLOSION(rand() % 6), 200);
              //play_sample(s_expl[rand() % 6], 200, 127, 800 + rand() % 400, 0);
            playcount = PLAYDELAY;
            createExplosion(world.bullets[x].x, world.bullets[x].y, &world);
            if (world.bullets[x].bulletType == BULLET_TYPE_CLUSTER)
            {
              world.bullets[x].x = ((int)(bullet_orig_x / TILESIZE)) * TILESIZE + HALFTILESIZE;
              world.bullets[x].y = ((int)(bullet_orig_y / TILESIZE)) * TILESIZE + HALFTILESIZE;
              while (checkFlagsAt(&world, (int)world.bullets[x].x, (int)world.bullets[x].y, TILE_IS_WALL))
              {
                  world.bullets[x].x -= 5 * world.bullets[x].dx;
                  world.bullets[x].y -= 5 * world.bullets[x].dy;
              }
              createClusterExplosion(&world, world.bullets[x].x, world.bullets[x].y, 16, cluster_strength, PLAYER_ID);
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
                createClusterExplosion(&world, world.plr.x, world.plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, PLAYER_ID);
                if ((world.gameModifiers & GAMEMODIFIER_OVERPOWERED_POWERUPS) != 0)
                {
                  createClusterExplosion(&world, world.plr.x, world.plr.y, 16, difficulty == DIFFICULTY_BRUTAL ? 3 : 4, PLAYER_ID);
                }
            }
            if (playcount == 0)
//              play_sample(s_expl[rand() % 6], 200, 127, 800 + rand() % 400, 0);
                triggerSample(SAMPLE_EXPLOSION(rand() % 6), 200);
            playcount = PLAYDELAY;
            createShadeAroundHitPoint(world.plr.x, world.plr.y, 9, &world);
            createExplosion(world.plr.x, world.plr.y, &world);
            if (world.plr.health <= 0)
            {
              createExplosion(world.plr.x - 20, world.plr.y - 20, &world);
              createExplosion(world.plr.x + 20, world.plr.y + 20, &world);
              createExplosion(world.plr.x - 20, world.plr.y + 20, &world);
              createExplosion(world.plr.x + 20, world.plr.y - 20, &world);
              //UniqueGameData.deaths++;
              //StoreUniqueGame(&UniqueGameData);

              chunkrest(1); // The death sample won't play else
//              play_sample(s_die[rand() % 6], 255, 127 + (world.enm[y].x - 240) / 8, 900 + rand() % 200, 0);
              triggerSampleWithParams(SAMPLE_DEATH(rand() % 6), 255, 127 + (world.enm[y].x - 240) / 8, 900 + rand() % 200);
              world.plr.reload = 100;
              break;
            }
          }
          int deathsample_plays = 0;
          if (world.bullets[x].hurtsMonsters)
            for (y = 0; y < ENEMYCOUNT; y++)
            {
              if (world.enm[y].id == NO_OWNER || world.enm[y].id >= 9000 || world.enm[y].roomid != world.currentRoom)
                continue;

              if (bullet_hit(world.enm + y, world.bullets + x))
              {
                createShadeAroundHitPoint(world.enm[y].x, world.enm[y].y, 9, &world);
                createExplosion(world.enm[y].x, world.enm[y].y, &world);
                createExplosion(world.enm[y].x, world.enm[y].y, &world);
                createExplosion(world.enm[y].x, world.enm[y].y, &world);
                if (!deathsample_plays)
                  if (playcount == 0)
                    triggerSample(SAMPLE_EXPLOSION(rand() % 6), 200);
                    //play_sample(s_expl[rand() % 6], 200, 127, 800 + rand() % 400, 0);
                    
                if (world.bullets[x].bulletType == BULLET_TYPE_CLUSTER)
                {
                    createClusterExplosion(&world, world.bullets[x].x, world.bullets[x].y, 16, cluster_strength, PLAYER_ID);
                }
                    
                playcount = PLAYDELAY;
                if (world.enm[y].id == NO_OWNER) // enemy was killed (bullet_hit has side effects)
                {
                  //world.floorShadeMap[world.currentRoom][world.enm[y].x / TILESIZE][world.enm[y].y / TILESIZE] |= FLOOR_SHADE_MAP_FLAG_BLOOD;
                  setTileFlag(&world, world.enm[y].x, world.enm[y].y, TILE_IS_BLOOD_STAINED);
                  //UniqueGameData.kills++;
                  world.plr.gold += world.enm[y].gold;

                  if (world.enm[y].gold > 0)
                  {
                    sprintf(hintText, "+ %d", world.enm[y].gold);
                    hintX = world.enm[y].x - 15;
                    hintY = world.enm[y].y - 15;
                    hintDim = 6;
                    hint_timeShows = 40;
                  }
                  if((world.gameModifiers & GAMEMODIFIER_MULTIPLIED_GOLD) == 0)
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
                    boss_logic(&world, &bossWantToShoot, &boss_waypoint, 1);
                    chunkrest(1);
//                    play_sample(s_bosstalk2, 255, 127 + (world.enm[y].x - 240) / 8, 1000, 0);
                    triggerSampleWithParams(SAMPLE_BOSSTALK_2, 255, 127 + (world.enm[y].x - 240) / 8, 1000);
                  }
                  else
                  {
                    chunkrest(1); // The death sample won't play else
                    if (!deathsample_plays)
//                      play_sample(s_die[rand() % 6], 255, 127 + (world.enm[y].x - 240) / 8, 900 + rand() % 200, 0);
                     triggerSampleWithParams(SAMPLE_DEATH(rand() % 6), 255, 127 + (world.enm[y].x - 240) / 8, 900 + rand() % 200);
                  }
                  deathsample_plays = 1;
                  spawnBodyParts(&world.enm[y]);
                }
                break;
              }
            }
        }
        if (world.bullets[x].bulletType == BULLET_TYPE_NORMAL)
        {
            int bullet_sprite = ((int)(world.bullets[x].x + world.bullets[x].y) / 30) % 4;
            masked_blit(world.spr, world.buf, 140 + bullet_sprite * 10, 140, world.bullets[x].x - 5, world.bullets[x].y - 5, 10, 10);
        } else if (world.bullets[x].bulletType == BULLET_TYPE_CLUSTER)
        {
            masked_blit(world.spr, world.buf, 140, 150, world.bullets[x].x - 6, world.bullets[x].y - 6, 13, 12);
        }
      }

    draw_map(&world, mission % 3 + 1);

    drawPlayerLegend(&world);
    
    completetime++;

    // Draw hint

    if (hint_timeShows > 0)
    {
      hint_timeShows--;
      int hint_col = hint_timeShows * hintDim;
      textprintf_ex(world.buf, font, hintX, hintY, GRAY(hint_col), -1, hintText);
    }
    if (world.bossFight && ++worldScriptFrameCount >= 3)
    {
      worldScriptFrameCount = 0;
      boss_logic(&world, &bossWantToShoot, &boss_waypoint, 0);
    }
    /*if (world.scripting && ++worldScriptFrameCount >= 10)
    {
      worldScriptFrameCount = 0;

      Enemy *boss = NULL;
      if (world.bossFight)
      {
        for (x = 0; x < ENEMYCOUNT; x++)
        {
          if (world.enm[x].type == ARCH_MAGE)
          {
            boss = &world.enm[x];
            break;
          }
        }
      }
      int inSameRoom = boss != NULL && boss->roomid == world.currentRoom;
      double seesPlayer = (inSameRoom && sees_each_other(boss, &world.plr, &world)) ? 1 : 0;
      setMemScriptParameter(&world.worldScript, "sees-player", seesPlayer);
      setMemScriptParameter(&world.worldScript, "health", inSameRoom ? boss->health : -1);
      setMemScriptParameter(&world.worldScript, "my-x", inSameRoom ? boss->x : -1);
      setMemScriptParameter(&world.worldScript, "my-y", inSameRoom ? boss->y : -1);
      setMemScriptParameter(&world.worldScript, "player-x", world.plr.x);
      setMemScriptParameter(&world.worldScript, "player-y", world.plr.y);
      setMemScriptParameter(&world.worldScript, "player-room", world.currentRoom);
      setMemScriptParameter(&world.worldScript, "difficulty", difficulty);
      if (inSameRoom)
      {
        runMemScript(&world.worldScript, "progress-frame", NULL);
        boss->health = getMemScriptParameter(&world.worldScript, "health");
      }
      else
      {
        runMemScript(&world.worldScript, "progress-frame-not-in-same-room", NULL);
      }
      bossMovementStrategy = runMemScript(&world.worldScript, "movement-strategy", NULL);
      bossWantToShoot = runMemScript(&world.worldScript, "want-to-shoot", NULL);

      setMemScriptParameter(&world.worldScript, "player-health", world.plr.health);
      setMemScriptParameter(&world.worldScript, "player-magic", world.plr.ammo);
      setMemScriptParameter(&world.worldScript, "player-gold", world.plr.gold);
      runMemScript(&world.worldScript, "modify-player", NULL);
      world.plr.health = getMemScriptParameter(&world.worldScript, "player-health");
      world.plr.ammo = getMemScriptParameter(&world.worldScript, "player-magic");
      world.plr.gold = getMemScriptParameter(&world.worldScript, "player-gold");

      int wantToModifyTerrain = runMemScript(&world.worldScript, "want-to-modify-terrain", NULL);
      int modMapX = 0, modMapY = 0, modMapType = 0;
      int wantMore;
      if (wantToModifyTerrain)
      {
        do
        {
          setMemScriptParameter(&world.worldScript, "x", modMapX);
          setMemScriptParameter(&world.worldScript, "y", modMapY);
          setMemScriptParameter(&world.worldScript, "type", modMapType);
          wantMore = runMemScript(&world.worldScript, "modify-terrain", NULL);

          modMapX = getMemScriptParameter(&world.worldScript, "x");
          modMapY = getMemScriptParameter(&world.worldScript, "y");
          modMapType = getMemScriptParameter(&world.worldScript, "type");
          if (wantMore != 1)
          {
            if (modMapType == 1)
            {
              world.map[modMapX][modMapY] = createTile(TILE_SYM_FLOOR);
            }
            else if (modMapType == 2)
            {
              world.map[modMapX][modMapY] = createTile(TILE_SYM_WALL1);
            }
            else if (modMapType == 3)
            {
              world.map[modMapX][modMapY] = createTile(TILE_SYM_EXIT_LEVEL);
            }
            if (modMapType > 0)
            {
              triggerSample(SAMPLE_EXPLOSION(rand() % 6), 200);
             // play_sample(s_expl[rand() % 6], 200, 127, 800 + rand() % 400, 0);
              createExplosion(modMapX * TILESIZE + TILESIZE / 2, modMapY * TILESIZE + TILESIZE / 2, &world);
              createExplosion(modMapX * TILESIZE + TILESIZE / 2, modMapY * TILESIZE + TILESIZE / 2, &world);
              createExplosion(modMapX * TILESIZE + TILESIZE / 2, modMapY * TILESIZE + TILESIZE / 2, &world);
            }
          }

          int flags = world.map[modMapX][modMapY].flags;
          if (flags & TILE_IS_WALL)
          {
            modMapType = 2;
          }
          else if (flags & TILE_IS_EXIT_LEVEL)
          {
            modMapType = 3;
          }
          else if (flags & TILE_IS_EXIT_POINT)
          {
            modMapType = 4;
          }
          else
          {
            modMapType = 1;
          }

          if (wantMore == 1)
          {
            if ((int)(world.plr.x / TILESIZE) == modMapX && (int)(world.plr.y / TILESIZE) == modMapY)
            {
              modMapType = 11;
            }
            else for (int i = 0; i < ENEMYCOUNT; i++)
            {
              Enemy *e = &world.enm[i];
              if (e->id != NO_OWNER && (int)(e->x / TILESIZE) == modMapX && (int)(e->y / TILESIZE) == modMapY)
              {
                modMapType = 10;
                break;
              }
            }
            wantMore = -2;
          }

        } while (wantMore == -2);
      }

      int wantToSpawnMonsters = runMemScript(&world.worldScript, "want-to-spawn-monsters", NULL);
      if (wantToSpawnMonsters)
      {
        do
        {
          wantMore = runMemScript(&world.worldScript, "spawn-monster", NULL);
          modMapX = getMemScriptParameter(&world.worldScript, "x");
          modMapY = getMemScriptParameter(&world.worldScript, "y");
          modMapType = getMemScriptParameter(&world.worldScript, "type");
          if (modMapType > 0)
          {
            spawnEnemy(modMapX, modMapY, modMapType + 200 - 1, world.currentRoom, &world);
          }
        } while (wantMore == -2);
      }

      int wantToSpawnFireballs = runMemScript(&world.worldScript, "want-to-spawn-fire-balls", NULL);
      if (wantToSpawnFireballs)
      {
        do
        {
          wantMore = runMemScript(&world.worldScript, "spawn-fire-ball", NULL);
          modMapX = getMemScriptParameter(&world.worldScript, "x");
          modMapY = getMemScriptParameter(&world.worldScript, "y");
          double dx = getMemScriptParameter(&world.worldScript, "direction-x");
          double dy = getMemScriptParameter(&world.worldScript, "direction-y");
          double dirNormalize = 1 / sqrt((dx * dx) + (dy * dy));
          dx *= dirNormalize;
          dy *= dirNormalize;
          shootOneShotAtXy(modMapX, modMapY, dx, dy, boss->id, 0, &world);
        } while (wantMore == -2);
      }

      int showText = runMemScript(&world.worldScript, "show-text", NULL);
      if (showText > -1 && world.worldScript.strings)
      {
        hintX = 5;
        hintY = 5;
        hintDim = 6;
        hint_timeShows = 120;
        strcpy(hintText, world.worldScript.strings[showText].str);
      }
      //world.plr.ammo = 15;
    }*/

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
/*      circle(world.buf, world.plr.x + TILESIZE * sin(completetime * 0.1), 
                        world.plr.y + TILESIZE * cos(completetime * 0.1), 
                        (1 - sin(completetime * 0.2)) * 3, makecol(0, 166, 0));
      circle(world.buf, world.plr.x - TILESIZE * sin(completetime * 0.1), 
                        world.plr.y - TILESIZE * cos(completetime * 0.1), 
                        (1 - cos(completetime * 0.2)) * 3, makecol(0, 0, 166));*/
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
    
    if (flyInTextX > -400)
    {
      textprintf_ex(world.buf, font, flyInTextX, 170, GRAY(255), -1, flyInText);
      if (flyInTextX > world.buf->w / 8 * 3 && flyInTextX < world.buf->w / 8 * 5)
      {
        flyInTextX -= 4;
      }
      else
      {
        flyInTextX -= 10;
      }
    }

    int offset = 2 * vibrations - rand() % (1 + 2 * vibrations);
    if (world.plr.health > 0)
    {
      stretch_blit(world.buf, screen, 0, 0, 480, 360, offset + screenHOffset, offset + screenVOffset, screenWidthScaled, screenHeightScaled);
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
      stretch_blit(world.buf, screen, startx, starty, endx, endy, screenHOffset, screenVOffset, screenWidthScaled, screenHeightScaled);
      chunkrest(20);
      if (world.plr.reload <= 0)
        break;
    }

    //playMP3();
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

      hint_timeShows = 0;
      int switchLevel = menu(1, &plrautosave, &mission, gameModifiers);
      if (switchLevel)
      {
        break;
      }
      drawStaticBackground();
    }
  }
  
  if (recordMode == RECORD_MODE_RECORD)
  {
    produceAndWriteContinuousData(keyPresses, keyPressBufferIdx + 1, &keyPressBufferIdx, fKeyPresses, timeStamp, 999, 999);
    fclose(fKeyPresses);
  }

  /*if (world.worldScriptInited)
  {
    freeMemScript(&world.worldScript);
  }*/

  destroy_bitmap(world.buf);
  destroy_bitmap(world.spr);
  destroy_bitmap(world.explosSpr);
  destroy_bitmap(bmp_levclear);
  /*destroy_sample(s_throw);
  destroy_sample(s_sel);

  destroy_sample(s_bosstalk1);
  destroy_sample(s_bosstalk2);

  for (i = 0; i < 6; i++)
  {
    destroy_sample(s_expl[i]);
    destroy_sample(s_die[i]);
  }*/

  return mission;
}

void initAllegro()
{
  srand((int)time(NULL));
  allegro_init();
  install_timer();

  set_color_depth(32);
  install_keyboard();
  //install_mouse();

  if (install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL) != 0)
  {
    allegro_message("Error initialising sound system.\n%s\n", allegro_error);
    exit(1);
  }

  int w = gameSettings.screenWidth;
  int h = gameSettings.screenHeight;
  int fullScreen = gameSettings.screenMode == 1;
  if (fullScreen)
  {
   printf("starting in fullscreen\n");
   fullScreen = !set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, w, h, 0, 0);
  }
  if (!fullScreen)
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
