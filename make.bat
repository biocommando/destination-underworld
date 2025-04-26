for /F "tokens=*" %%i in ('type environment.txt') do set %%i

set VERSION=1.0
git log --format=%%h -n 1 > version_info.txt
node gen_version.js %VERSION%
del version_info.txt

: compile the sha1 module as separate step as it's a 3rd party component and we're not so interested
: in the compiler warnings

%compiler% %compiler_flags% -w -c src/sha1/sha1.c

%compiler%  %auth_setup% .\src\create_mission_pack_auth_file.c .\src\sha1\sha1.o .\src\sha1\du_dmac.c -o mpauth

cd src/synth
%compiler% %compiler_flags% -Ofast -c *.c
cd ../..
cd src
%compiler% %compiler_flags% %auth_setup% -c *.c sha1/du_dmac.c ^ -I..\%allegro_path%\include
cd ..
set objfiles=sha1.o ^
    src/du_dmac.o ^
    src/arenaconf.o ^
    src/bossfightconf.o ^
    src/game_playback.o ^
    src/gamePersistence.o ^
    src/helpers.o ^
    src/keyhandling.o ^
    src/menu.o ^
    src/predictableRandom.o ^
    src/renderWorld.o ^
    src/sampleRegister.o ^
    src/settings.o ^
    src/world.o ^
    src/worldInteraction.o ^
    src/vfx.o ^
    src/read_level.o ^
    src/allegro_management.o ^
    src/loadindicator.o ^
    src/duscript.o ^
    src/sprites.o ^
    src/record_file.o ^
    src/best_times.o ^
    src/midi_playback.o ^
    src/boss_logic.o ^
    src/bullet_logic.o ^
    src/enemy_logic.o ^
    src/potion_logic.o ^
    src/screenshot.o ^
    src/game.o ^
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
    src/synth/synth.o

%compiler% %compiler_flags% %auth_setup% %objfiles% src/main.o ^
    %allegro_path%\lib\liballegro_monolith.dll.a ^
    -o DestinationUnderworld.exe

%compiler% test/unittests.c -I test %objfiles% ^
     src/synth/test/*.c ^
     src/test/*.c ^
     %allegro_path%\lib\liballegro_monolith.dll.a ^
    -o unittests.exe