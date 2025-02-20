@echo off
gcc test\test-du.c -o test-du.exe
.\test-du.exe -ctest\normal-complete-state.dat ^
-rtest\normal-recording.dat ^
-ctest\explosion-madness-complete-state.dat ^
-rtest\explosion-madness-recording.dat ^
-ctest\brutally-hard-complete-state.dat ^
-rtest\brutally-hard-recording.dat ^
-ctest\overpowerup-complete-state.dat ^
-rtest\overpowerup-recording.dat ^
-ctest\potion-only-complete-state.dat ^
-rtest\potion-only-recording.dat ^
-ctest\powerup-only-complete-state.dat ^
-rtest\powerup-only-recording.dat ^
-ctest\perks-change-mid-level-complete-state.dat ^
-rtest\perks-change-mid-level-recording.dat ^
-ctest\boss-fight-complete-state.dat ^
-rtest\boss-fight-recording.dat

echo ~~ Test synth module ~~
cd src\synth
call test_synth.bat