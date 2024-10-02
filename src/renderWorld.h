#pragma once

#include "world.h"

void draw_enemy(Enemy *enm, World *world);
void draw_player_legend(World *world, int x, int y);
void draw_map(World *world, int draw_walls, int vibration_intensity);
void move_and_draw_body_parts(World *world);
int progress_and_draw_explosions(World *world);
void progress_and_draw_sparkles(World *world);
void display_level_info(World *world, int mission, int mission_count, long completetime);
void show_gold_hint(World *world, int number);
void show_ingame_info_screen(World *world);
