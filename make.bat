for /F "tokens=*" %%i in ('type environment.txt') do set %%i

set VERSION=1.0
git log --format=%%h -n 1 > version_info.txt
node gen_version.js %VERSION%
del version_info.txt

@rem compile the sha1 module as separate step as it's a 3rd party component and we're not so interested
@rem in the compiler warnings

%compiler% %compiler_flags% -w -c src/sha1/sha1.c

%compiler%  %auth_setup% .\src\create_mission_pack_auth_file.c .\src\sha1\sha1.o .\src\sha1\du_dmac.c -o mpauth

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
src/synth/adsr_envelope.c ^
src/synth/basic_delay.c ^
src/synth/basic_oscillator.c ^
src/synth/envelope_stage.c ^
src/synth/wav_handler.c ^
src/synth/wt_sample_loader.c ^
src/synth/midi_player.c ^
src/synth/midi_reader.c ^
src/synth/moog_filter.c ^
src/synth/synth_random.c ^
src/synth/synth.c ^
-I%allegro_path%\include ^
%allegro_path%\lib\liballegro_monolith.dll.a ^
-o DestinationUnderworld.exe