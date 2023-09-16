#pragma once

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_audio.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define rectfill(x, y, x2, y2, col) al_draw_filled_rectangle(x, y, x2 + 1, y2 + 1, col)
#define circlefill al_draw_filled_circle

#define clear_to_color(x) al_clear_to_color(x)

#define AL_PI 3.141592653589793

#define makecol al_map_rgb

#define SAMPLE ALLEGRO_SAMPLE
#define load_sample al_load_sample
#define destroy_sample al_destroy_sample
#define play_sample(data, gain, pan, speed, loop, id) al_play_sample(data, gain, pan, speed, loop ? ALLEGRO_PLAYMODE_LOOP : ALLEGRO_PLAYMODE_ONCE, id)

#define BITMAP ALLEGRO_BITMAP
#define load_bitmap al_load_bitmap
#define destroy_bitmap al_destroy_bitmap
#define MASKED_BITMAP(x) al_convert_mask_to_alpha(x, al_map_rgb(255, 0, 255))

int wait_event();

int init_allegro();
void wait_delay(int v);
void wait_delay_ms(int ms);
void wait_key_press(int key);
int check_key(int key);

ALLEGRO_FONT *get_font();

#define textprintf_ex(x, y, col, unused, text, ...) al_draw_textf(get_font(), col, x, y, 0, text, __VA_ARGS__)