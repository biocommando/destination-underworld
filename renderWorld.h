#ifndef RENDER_WORLD_H
#define RENDER_WORLD_H
#include "world.h"

void draw_enemy(Enemy *enm, World *world);
void drawPlayerLegend(World *world);
void draw_map(World *world, int col);
void moveAndDrawBodyParts(World *world);
int progressAndDrawExplosions(World *world);
void displayLevelInfo(World *world, int mission, int missionCount, BITMAP *bmp_levclear, FONT *font);
void show_gold_hint(World *world, char *hintText, int *hintX, int *hintY, int *hintDim, int *hint_timeShows, int number);

#endif
