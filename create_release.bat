rd /s /q DestinationUnderworldRelease
call make.bat

mkdir DestinationUnderworldRelease
mkdir DestinationUnderworldRelease\dataloss
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

xcopy dataloss\sel.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\warp.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\bt1.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\bt2.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\select_weapon.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\throw.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\healing.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\rune_of_protection.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\turret.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\blast.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\spawn.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_shield.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_stop.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_fast.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_boost.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\potion_heal.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\ex1.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\ex2.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\ex3.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\ex4.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\ex5.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\ex6.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\die1.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\die2.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\die3.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\die4.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\die5.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\die6.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\wet_1.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\wet_2.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\wet_3.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\menusel.wav DestinationUnderworldRelease\dataloss
xcopy dataloss\menuchg.wav DestinationUnderworldRelease\dataloss

xcopy dataloss\music1.mp3 DestinationUnderworldRelease\dataloss
xcopy dataloss\music2.mp3 DestinationUnderworldRelease\dataloss
xcopy dataloss\music3.mp3 DestinationUnderworldRelease\dataloss
xcopy dataloss\music4.mp3 DestinationUnderworldRelease\dataloss

xcopy allegro_monolith-5.2.dll DestinationUnderworldRelease
xcopy DestinationUnderworld.exe DestinationUnderworldRelease