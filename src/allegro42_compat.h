#pragma once

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_audio.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define rectfill(x, y, x2, y2, col) al_draw_filled_rectangle(x, y, x2 + 1, y2 + 1, col)

#define AL_PI 3.141592653589793

int wait_event();

int init_allegro();
void wait_delay(int v);
void wait_delay_ms(int ms);
void wait_key_press(int key);
int wait_key_presses(const int *keys, int num_keys);
void wait_key_release(int key);
int check_key(int key);

ALLEGRO_FONT *get_font();