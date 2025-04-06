for /F "tokens=*" %%i in ('type environment.txt') do set %%i

set VERSION=1.0
git log --format=%%h -n 1 > version_info.txt
node gen_version.js %VERSION%
del version_info.txt

@rem compile the sha1 module as separate step as it's a 3rd party component and we're not so interested
@rem in the compiler warnings

%compiler% %compiler_flags% -w -c src/sha1/sha1.c

%compiler%  %auth_setup% .\src\create_mission_pack_auth_file.c .\src\sha1\sha1.o .\src\sha1\du_dmac.c -o mpauth

cd src/synth
%compiler% %compiler_flags% -Ofast -c *.c
cd ../..

%compiler% %compiler_flags% %auth_setup% ^
sha1.o ^
src/sha1/du_dmac.c ^
src/arenaconf.c ^
src/bossfightconf.c ^
src/game_playback.c ^
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
src/vfx.c ^
src/read_level.c ^
src/allegro_management.c ^
src/loadindicator.c ^
src/duscript.c ^
src/sprites.c ^
src/record_file.c ^
src/best_times.c ^
src/midi_playback.c ^
src/boss_logic.c ^
src/bullet_logic.c ^
src/enemy_logic.c ^
src/potion_logic.c ^
src/screenshot.c ^
src/game.c ^
src/synth/adsr_envelope.o ^
src/synth/basic_delay.o ^
src/synth/basic_oscillator.o ^
src/synth/envelope_stage.o ^
src/synth/wav_handler.o ^
src/synth/wt_sample_loader.o ^
src/synth/midi_player.o ^
src/synth/midi_reader.o ^
src/synth/moog_filter.o ^
src/synth/synth_random.o ^
src/synth/synth.o ^
-I%allegro_path%\include ^
%allegro_path%\lib\liballegro_monolith.dll.a ^
-o DestinationUnderworld.exe