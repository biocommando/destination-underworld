#pragma once

#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_font.h"
#include "allegro5/allegro_primitives.h"
#include "allegro5/allegro_audio.h"

/*
 * Handle all pending events (other than timer events) from allegro event queue.
 * Internally handles key up / down events (so that the key states
 * can be just checked using the check_key function) and music playback.
 */
void consume_event_queue();

/*
 * Wait for a timer event from allegro event queue.
 */
void wait_timer_event();

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
 * Get the first key pressed in the key buffer, or 0 if none is pressed.
 */
int get_key();

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

/*
 * Allocate a new bitmap that contains the screen's contents.
 */
ALLEGRO_BITMAP *get_screen();
