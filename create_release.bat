for /F "tokens=*" %%i in ('type environment.txt') do set %%i

rd /s /q DestinationUnderworldRelease
call make.bat

mkdir DestinationUnderworldRelease
mkdir DestinationUnderworldRelease\dataloss
mkdir DestinationUnderworldRelease\dataloss\midi-music
mkdir DestinationUnderworldRelease\dataloss\core-pack
mkdir DestinationUnderworldRelease\dataloss\editor

xcopy dataloss\core-pack\mission* DestinationUnderworldRelease\dataloss\core-pack
xcopy dataloss\core-pack\best_times.dat DestinationUnderworldRelease\dataloss\core-pack
xcopy dataloss\core-pack\enemy-properties.dat DestinationUnderworldRelease\dataloss\core-pack
xcopy dataloss\core-pack\arenas.dat DestinationUnderworldRelease\dataloss\core-pack
xcopy dataloss\core-pack\help.dat DestinationUnderworldRelease\dataloss\core-pack

xcopy dataloss\editor DestinationUnderworldRelease\dataloss\editor

xcopy dataloss\sprites.dat DestinationUnderworldRelease\dataloss
xcopy dataloss\sounds.dat DestinationUnderworldRelease\dataloss
xcopy dataloss\settings.dat DestinationUnderworldRelease\dataloss

xcopy dataloss\sprites.png DestinationUnderworldRelease\dataloss
xcopy dataloss\hell.jpg DestinationUnderworldRelease\dataloss

xcopy dataloss\sel.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\warp.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\bt1.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\bt2.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\select_weapon.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\throw.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\healing.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\rune_of_protection.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\turret.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\blast.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\spawn.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_shield.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_stop.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_fast.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_boost.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_heal.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\ex1.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\ex2.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\ex3.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\ex4.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\ex5.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\ex6.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\die1.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\die2.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\die3.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\die4.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\die5.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\die6.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\wet_1.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\wet_2.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\wet_3.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\menusel.ogg DestinationUnderworldRelease\dataloss
xcopy dataloss\menuchg.ogg DestinationUnderworldRelease\dataloss

xcopy dataloss\midi-music\*.mid DestinationUnderworldRelease\dataloss\midi-music
xcopy dataloss\midi-music\*.ini DestinationUnderworldRelease\dataloss\midi-music
node .\dataloss\midi-music\track_name_mapper.js DestinationUnderworldRelease\dataloss\midi-music

xcopy dataloss\wt_sample_slot_*.wav DestinationUnderworldRelease\dataloss

xcopy %allegro_path%\bin\allegro_monolith-5.2.dll DestinationUnderworldRelease
xcopy DestinationUnderworld.exe DestinationUnderworldRelease