#pragma once

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_audio.h"

#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

// Legacy compatibility
#define rectfill(x, y, x2, y2, col) al_draw_filled_rectangle(x, y, x2 + 1, y2 + 1, col)

// Legacy compatibility
#define AL_PI 3.141592653589793

/*
 * Wait for event from allegro event queue and handle the event.
 * Internally handles key up / down events (so that the key states
 * can be just checked using the check_key function) and music playback.
 *
 * Returns 0 if waiting fails, 1 if event type is ALLEGRO_EVENT_TIMER and 2
 * otherwise.
 */
int wait_event();

/*
 * Initializes allegro library. Also initializes the midi playback engine.
 */
int init_allegro();

/*
 * Frees all the memory reserved by the init_allegro function.
 */
void destroy_allegro();

/*
 * Waits for v "ticks".
 */
void wait_delay(int v);

/*
 * Waits for ms milliseconds. Note that wait resolution is "tick" which
 * is 10 ms.
 */
void wait_delay_ms(int ms);

/*
 * Hang execution (not busy loop) until the key is pressed and released.
 */
void wait_key_press(int key);

/*
 * Hang execution (not busy loop) until any of the provided keys are pressed and released.
 */
int wait_key_presses(const int *keys, int num_keys);

/*
 * Wait until a key up event is received for the key.
 */
void wait_key_release(int key);

/*
 * Check if the key has been pressed but not yet released.
 */
int check_key(int key);

/*
 * Returns the key pressed if one of the provided keys is pressed, otherwise 0.
 * If multiple keys are pressed, returns the first one present in the array.
 */
int check_keys(const int *keys, int num_keys);

/*
 * Get the font for in-game texts.
 */
ALLEGRO_FONT *get_font();

/*
 * Get the font for in-game texts but in smaller size.
 */
ALLEGRO_FONT *get_font_tiny();

/*
 * Get the font for menus.
 */
ALLEGRO_FONT *get_menu_font();

/*
 * Get a large font for menu titles.
 */
ALLEGRO_FONT *get_menu_title_font();