#pragma once

#define MAPMAX_X 16
#define MAPMAX_Y 12
#define TILESIZE 30
#define HALFTILESIZE 15
#define THIRDTILESIZE 10
#define ANIM_FRAME_COUNT 40
#define TO_PIXEL_COORDINATES(point) ((point) * TILESIZE + HALFTILESIZE)

#define WITH_SANITIZED_NS_TILE_COORDINATES(x, y) \
    const unsigned ok_x = x;             \
    const unsigned ok_y = y;             \
    if (ok_x < MAPMAX_X && ok_y < MAPMAX_Y)

#define WITH_SANITIZED_TILE_COORDINATES(x, y) \
    WITH_SANITIZED_NS_TILE_COORDINATES(x / TILESIZE, y / TILESIZE)

#define BODYPARTCOUNT 32
#define ROOMCOUNT 8
#define ENEMY_TYPE_COUNT 5 // all "normal" enemies excluding the special boss type
#define ENEMYCOUNT 200
#define EXPLOSIONCOUNT 32
// Old value for EXPLOSIONCOUNT was 16
#define MAX_VIBRATIONS 16
#define FLAME_FX_COUNT 256
#define EMBERS_PER_FLAME_FX 10

#define PLR_DIR_HELPER_INITIAL_INTENSITY 100

#define DATADIR "./dataloss/"
#define SAVE_FILENAME DATADIR "%s/save.dat"

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

#define AUTH_CTX_CONF 0
#define AUTH_CTX_GAMEPLAY_RECORDING 1
#define AUTH_CTX_SAVE_GAME 2
#define AUTH_CTX_COMPLETED_GAME_MODES 3

#define M_PI 3.14159265358979323846

// Special mission number where the player can change their loadout
#define LIMBO_MISSION 999