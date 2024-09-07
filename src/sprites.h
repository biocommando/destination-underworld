#pragma once

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"

#define SPRITE_ID_BULLET 0
#define SPRITE_ID_CLUSTER 1
#define SPRITE_ID_POTION 2
#define SPRITE_ID_POTION_BUBBLE 3
#define SPRITE_ID_HEALTH 4
#define SPRITE_ID_RUNE_OF_PROTECTION 5
#define SPRITE_ID_ENEMY 6
#define SPRITE_ID_AMMO 7
#define SPRITE_ID_TURRET 8
#define SPRITE_ID_SPARKLES 9
#define SPRITE_ID_PENTAGRAM_WALL 10
#define SPRITE_ID_CRACKED_WALL 11
#define SPRITE_ID_PLR_LEGEND_FONT 12
#define SPRITE_ID_POTION_DURATION 13
#define SPRITE_ID_BODY_PART 14
#define SPRITE_ID_POTION_EFFECT 15

#define SPRITE_ID_MIN SPRITE_ID_BULLET
#define SPRITE_ID_MAX SPRITE_ID_POTION_EFFECT

typedef struct
{
    int sx, sy, width, height;
} DuSprite;

int set_sprite(int sprite_id, DuSprite sprite);
DuSprite *get_sprite(int sprite_id);
int read_sprites_from_file(const char *filename, int min_id, int max_id);
void draw_sprite(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y);
void draw_sprite_centered(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y);
void draw_sprite_animated(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y, int frame_offset_x, int frame_offset_y);
void draw_sprite_animated_centered(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y, int frame_offset_x, int frame_offset_y);