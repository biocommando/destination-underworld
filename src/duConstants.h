#pragma once

#define MAPMAX_X 16
#define MAPMAX_Y 12
#define TILESIZE 30
#define HALFTILESIZE 15
#define THIRDTILESIZE 10
#define ANIM_FRAME_COUNT 40

#define BODYPARTCOUNT 32
#define ROOMCOUNT 8
#define ENEMY_TYPE_COUNT 5 // all "normal" enemies excluding the special boss type
#define ENEMYCOUNT 200
#define EXPLOSIONCOUNT 32
// Old value for EXPLOSIONCOUNT was 16
#define MAX_VIBRATIONS 16
#define BULLETCOUNT 600
#define SPARKLE_FX_COUNT 60
#define SPARKLE_FX_CIRCLE_COUNT 10
#define FLAME_FX_COUNT 64
#define EMBERS_PER_FLAME_FX 10
#define POTION_COUNT 40
#define POTION_PRESET_RANGE_START 0
#define POTION_PRESET_RANGE_END 16
#define POTION_DROP_RANGE_START POTION_PRESET_RANGE_END
#define POTION_DROP_RANGE_END POTION_COUNT

#define PLR_DIR_HELPER_INITIAL_INTENSITY 100

#define DATADIR ".\\dataloss\\"
#define SAVE_FILENAME DATADIR "%s\\save.dat"

#define DIFFICULTY_NORMAL 0
#define DIFFICULTY_BRUTAL 1

#define RECORD_MODE_NONE 0
#define RECORD_MODE_RECORD 1
#define RECORD_MODE_PLAYBACK 2

#define SAMPLE_WARP 1
#define SAMPLE_BOSSTALK_1 2
#define SAMPLE_BOSSTALK_2 3
#define SAMPLE_SELECT_WEAPON 5
#define SAMPLE_THROW 6

#define SAMPLE_HEAL 10
#define SAMPLE_PROTECTION 11
#define SAMPLE_TURRET 12
#define SAMPLE_BLAST 13

#define SAMPLE_SPAWN 14

#define SAMPLE_EXPLOSION(x) (1000 + (x))
#define SAMPLE_DEATH(x) (2000 + (x))

#define SAMPLE_POTION(x) (3000 + (x))

#define SAMPLE_SPLASH(x) (4000 + (x))

#define SAMPLE_MENU_SELECT 5001
#define SAMPLE_MENU_CHANGE 5002

#define SCREEN_W 480
#define SCREEN_H 360

#define DISPLAY_W (SCREEN_W * 3)
#define DISPLAY_H (SCREEN_H * 3)

#define POTION_ID_NONE -1
#define POTION_ID_SHIELD 0
#define POTION_ID_STOP 1
#define POTION_ID_FAST 2
#define POTION_ID_BOOST 3
#define POTION_ID_HEAL 4
#define POTION_ID_MINOR_SHIELD 5
#define POTION_ID_INSTANT_HEAL 6