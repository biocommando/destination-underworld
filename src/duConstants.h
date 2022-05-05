#ifndef DUCONSTANTS_H
#define DUCONSTANTS_H

#define NO_OWNER 0
#define PLAYER_ID 9999
#define MAPMAX_X 16
#define MAPMAX_Y 12
#define TILESIZE 30
#define HALFTILESIZE 15
#define THIRDTILESIZE 10
#define PLAYDELAY 1
#define ANIM_FRAME_COUNT 40

#define BODYPARTCOUNT 32
#define ROOMCOUNT 8
#define ENEMYCOUNT 200
#define EXPLOSIONCOUNT 16
#define BULLETCOUNT 400
#define SPARKLE_FX_COUNT 60

#define PLR_DIR_HELPER_INITIAL_INTENSITY 100

#define SAVE_FILENAME ".\\dataloss\\%s\\save%d.dat"
#define MENU_BITMAP_FILENAME ".\\dataloss\\menu.bmp"
#define HELP_BITMAP_FILENAME ".\\dataloss\\reference_full.bmp"
#define MENU_SAMPLE_FILENAME ".\\dataloss\\menuchg.wav"
#define MENU_SELECT_SAMPLE_FILENAME ".\\dataloss\\menusel.wav"
#define MENU_EXPLODE_FILENAME ".\\dataloss\\ex4.wav"
#define FONT_FILENAME ".\\dataloss\\gamefont.pcx"

#define DIFFICULTY_NORMAL 0
#define DIFFICULTY_BRUTAL 1

#define RECORD_MODE_NONE 0
#define RECORD_MODE_RECORD 1
#define RECORD_MODE_PLAYBACK 2

#define SAMPLE_SELECT 0
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

#endif
