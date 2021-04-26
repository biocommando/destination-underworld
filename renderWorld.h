#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H
#include "world.h"

void draw_enemy(Enemy *enm, World *world);
void draw_player_legend(World *world);
void draw_map(World *world, int col);
void move_and_draw_body_parts(World *world);
int progress_and_draw_explosions(World *world);
void display_level_info(World *world, int mission, int mission_count, BITMAP *bmp_levclear, FONT *font);
void show_gold_hint(World *world, int number);

#endif
