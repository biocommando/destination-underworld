#include "allegro42_compat.h"
#include "allegro5/allegro_acodec.h"
#include "duConstants.h"
#include "duMp3.h"
#include "gen_version_info.h"
#include <stdio.h>

char keybuffer[ALLEGRO_KEY_MAX];

ALLEGRO_FONT *font = NULL;
ALLEGRO_DISPLAY *display = NULL;
ALLEGRO_TIMER *timer = NULL;
ALLEGRO_EVENT_QUEUE *queue = NULL;
ALLEGRO_AUDIO_STREAM *audio_stream = NULL;
ALLEGRO_MIXER *mixer = NULL;
ALLEGRO_EVENT event;

int check_key(int key)
{
    if (key >= 0 && key < ALLEGRO_KEY_MAX)
    {
        return keybuffer[key];
    }
    return 0;
}

int wait_event()
{
    // Ensure that track is switched if track is played to the end while
    // waiting
    play_mp3();
    ALLEGRO_TIMEOUT tmo;
    al_init_timeout(&tmo, 0.1);
    int res = al_wait_for_event_until(queue, &event, &tmo);
    if (!res)
        return 0;
    if (event.type == ALLEGRO_EVENT_KEY_DOWN)
    {
        keybuffer[event.keyboard.keycode] = 1;
    }
    else if (event.type == ALLEGRO_EVENT_KEY_UP)
    {
        keybuffer[event.keyboard.keycode] = 0;
    }
    else if (event.type == ALLEGRO_EVENT_TIMER)
    {
        return 1;
    }
    return 2;
}

void wait_delay(int v)
{
    while (v > 0)
    {
        if (wait_event() == 1)
            v--;
    }
}

void wait_delay_ms(int ms)
{
    wait_delay(ms / 10);
}

void wait_key_press(int key)
{
	wait_key_presses(&key, 1);
}

int wait_key_presses(int *keys, int num_keys)
{
    int key = -1;
    while (key == -1)
    {
        for (int i = 0; i < num_keys; i++)
        {
            if (check_key(keys[i]))
            {
                key = keys[i];
            }
        }
        if (key == -1)
        {
            wait_event();
        }
    }
    wait_key_release(key);
    return key;
}

void wait_key_release(int key)
{
	while (keybuffer[key])
	{
		wait_event();
	}
}

int init_allegro()
{
    if (!al_init())
    {
        return 1;
    }
    memset(keybuffer, 0, sizeof(keybuffer));
    al_install_mouse();
    al_install_keyboard();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_acodec_addon();

    al_set_new_window_title("Destination Underworld " DU_VERSION);
    al_set_new_display_refresh_rate(60);
    al_set_new_display_flags(ALLEGRO_OPENGL);
    font = al_create_builtin_font();
    display = al_create_display(DISPLAY_W, DISPLAY_H);
    if (!display)
    {
        return 1;
    }

    al_install_audio();
    al_reserve_samples(64);

    timer = al_create_timer(1.0 / 100);
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_start_timer(timer);
    return 0;
}

ALLEGRO_FONT *get_font()
{
    return font;
}
