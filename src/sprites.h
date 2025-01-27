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
#define SPRITE_ID_SKELETON 16

#define SPRITE_ID_MIN SPRITE_ID_BULLET
#define SPRITE_ID_MAX SPRITE_ID_SKELETON

// A struct for sprite definition in a spritesheet.
typedef struct
{
    // Coordinates for the upper left corner of the sprite in the sprite sheet
    int sx, sy;
    // Dimensions for the sprite.
    int width, height;
} DuSprite;

/*
 * Set sprite definition in the sprite database with the given sprite_id.
 */
int set_sprite(int sprite_id, DuSprite sprite);
/*
 * Get sprite definition with the given id. If there's none with the id, returns NULL.
 */
DuSprite *get_sprite(int sprite_id);
/*
 * Reads sprites with ids min...max from provided file.
 * Uses the record file format with the following syntax for a single sprite:
 * sprite_[id] x=%d y=%d w=%d h=%d
 *
 * Note that the key values are just for human-readability, the key-value order needs to be exactly like this.
 */
int read_sprites_from_file(const char *filename, int min_id, int max_id);
/*
 * Draw a sprite from the sprite sheet on the screen. The x, y position is the upper left corner
 * of the sprite.
 */
void draw_sprite(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y);
/*
 * Draw a sprite from the sprite sheet on the screen. The x, y position is the center
 * of the sprite.
 */
void draw_sprite_centered(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y);
/*
 * Draw a sprite from the sprite sheet on the screen. The x, y position is the upper left corner
 * of the sprite.
 *
 * Uses x/y offset which means that the sprite sheet coordinates are shifted by frame times
 * width/height. This allows animating sprites with constant width/height without needing
 * to create multiple sprite definitions.
 */
void draw_sprite_animated(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y, int frame_offset_x, int frame_offset_y);
/*
 * Draw a sprite from the sprite sheet on the screen. The x, y position is the center
 * of the sprite.
 *
 * Uses x/y offset which means that the sprite sheet coordinates are shifted by frame times
 * width/height. This allows animating sprites with constant width/height without needing
 * to create multiple sprite definitions.
 */
void draw_sprite_animated_centered(ALLEGRO_BITMAP *sprite_sheet, int sprite_id, int x, int y, int frame_offset_x, int frame_offset_y);