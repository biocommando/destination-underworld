#pragma once

#define CMD_ARG_AND_DOC(arg, doc) \
    arg "\0" doc

#define ARG_HELP CMD_ARG_AND_DOC("help", "Display this help message")
#define ARG_PLAYER_DAMAGE CMD_ARG_AND_DOC("player-damage", "Set to 'off' to enable no damage cheat")
#define ARG_LOGGING CMD_ARG_AND_DOC("logging", "Set to 1 to enable logging")
#define ARG_REC_MODE CMD_ARG_AND_DOC("record-mode", "Gameplay recording mode: record or play")
#define ARG_REC_FILE_MODE CMD_ARG_AND_DOC("file", "Gameplay recording file for playback")
#define ARG_REC_NO_KEYPRESS CMD_ARG_AND_DOC("start-without-user-interaction", "Set to 1 so that user doesn't need to press any key to start gameplay recording playback.")
#define ARG_REC_AUTH CMD_ARG_AND_DOC("auth-hash-file", "Gameplay recording authentication file for playback")
#define ARG_DEF_GMODE CMD_ARG_AND_DOC("default-game-mode", "Set default game mode (integer, see source for mapping)")
#define ARG_SCREENSHOT CMD_ARG_AND_DOC("screenshot-buffer", "Set to 1 to enable screenshot capturing (capture key = insert)")
#define ARG_SETTINGS_DAT CMD_ARG_AND_DOC("settings-dat", "Filename (relative to work dir) to override settings.dat")

#define SETTING_MISSION_PACK "general", CMD_ARG_AND_DOC("mission-pack", "Mission pack to play (directory under data directory)")
#define SETTING_CUSTOM_RES "<mission-pack>", CMD_ARG_AND_DOC("custom-resources", "Use custom sprites and sounds? (1/0) (Load under mission pack directory.)")
#define SETTING_REQ_AUTH "<mission-pack>", CMD_ARG_AND_DOC("require-authentication", "Set to 0 to not require authentication for data files")

#define SETTING_VIBRATE "graphics", CMD_ARG_AND_DOC("vibration-mode", "Vibration intensity")
#define SETTING_FULLSCREEN "graphics", CMD_ARG_AND_DOC("fullscreen", "1=run in fullscreen, 0=windowed")
#define SETTING_MENU_FONT "graphics", CMD_ARG_AND_DOC("menu-font", "Font used for menu")
#define SETTING_GAME_FONT "graphics", CMD_ARG_AND_DOC("game-font", "Font used for game")

#define SETTING_MUSIC_ON "audio", CMD_ARG_AND_DOC("music-on", "1=music on, 0=music off")
#define SETTING_MUSIC_VOL "audio", CMD_ARG_AND_DOC("music-vol", "Music volume in range 0...1")
#define SETTING_SFX_VOL "audio", CMD_ARG_AND_DOC("sfx-vol", "Sound effects volume in range 0...1")
