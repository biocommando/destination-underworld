set VERSION=0.3
git log --format=%%h -n 1 > version_info.txt
node gen_version.js %VERSION%
del version_info.txt

gcc src/arenaconf.c ^
src/bossfightconf.c ^
src/game_playback.c ^
src/duMp3.c ^
src/musicTrack.c ^
src/gamePersistence.c ^
src/helpers.c ^
src/keyhandling.c ^
src/main.c ^
src/menu.c ^
src/predictableRandom.c ^
src/renderWorld.c ^
src/sampleRegister.c ^
src/settings.c ^
src/world.c ^
src/worldInteraction.c ^
src/allegro42_compat.c ^
src/loadindicator.c ^
src/duscript.c ^
src/sprites.c ^
src/record_file.c ^
src/best_times.c ^
-DENABLE_LOGGING ^
-DTRACE_LOG ^
-Iallegro\include ^
-O3 ^
allegro\lib\liballegro_monolith.dll.a ^
-o DestinationUnderworld.exe