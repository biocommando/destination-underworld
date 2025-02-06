#include "allegro42_compat.h"
#include "allegro5/allegro_acodec.h"
#include "duConstants.h"
#include "gen_version_info.h"
#include "midi_playback.h"
#include "settings.h"
#include <stdio.h>

static char keybuffer[ALLEGRO_KEY_MAX];

static ALLEGRO_FONT *font = NULL;
static ALLEGRO_DISPLAY *display = NULL;
static ALLEGRO_TIMER *timer = NULL;
static ALLEGRO_EVENT_QUEUE *queue = NULL;
static ALLEGRO_AUDIO_STREAM *audio_stream = NULL;
static ALLEGRO_EVENT event;

// The buffer size chosen to be as big as possible not to be
// very much audible when turning music off / switching tracks.
// As there's no realtime requirement, the latency can be high
#define AUDIO_BUFFER_SIZE 4096

int check_key(int key)
{
    if (key >= 0 && key < ALLEGRO_KEY_MAX)
    {
        return keybuffer[key];
    }
    return 0;
}

// Counter for adding a second of silence (or delay tail) after every midi file
static int midi_track_spacing_counter = 0;

int wait_event()
{
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
    else if (event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
    {
        if (!get_game_settings()->music_on)
            return 2;
        ALLEGRO_AUDIO_STREAM *stream = (ALLEGRO_AUDIO_STREAM *)event.any.source;
        float *buf = (float *)al_get_audio_stream_fragment(stream);
        if (buf)
        {
            MidiPlayer *mp = get_midi_player();
            mp->synth.total_volume = get_game_settings()->music_vol;
            midi_player_process_buffer(mp, buf, AUDIO_BUFFER_SIZE);
            al_set_audio_stream_fragment(stream, buf);
            if (mp->ended)
            {
                midi_track_spacing_counter += AUDIO_BUFFER_SIZE;
                if (midi_track_spacing_counter >= mp->sample_rate)
                {
                    midi_track_spacing_counter = 0;
                    next_midi_track(-1);
                }
            }
        }
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

int wait_key_presses(const int *keys, int num_keys)
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
    // al_install_mouse();
    al_install_keyboard();
    al_init_image_addon();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_acodec_addon();

    al_set_new_window_title("Destination Underworld " DU_VERSION);
    al_set_new_display_refresh_rate(60);
    int fullscreen_flag = get_game_settings()->fullscreen ? ALLEGRO_FULLSCREEN : 0;
    al_set_new_display_flags(ALLEGRO_OPENGL | fullscreen_flag);
    font = al_create_builtin_font();
    display = al_create_display(DISPLAY_W, DISPLAY_H);
    if (!display)
    {
        return 1;
    }

    al_install_audio();
    al_reserve_samples(64);

    audio_stream = al_create_audio_stream(4, AUDIO_BUFFER_SIZE, 44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2);
    ALLEGRO_MIXER *mixer = al_get_default_mixer();
    al_attach_audio_stream_to_mixer(audio_stream, mixer);

    init_midi_playback(44100);

    timer = al_create_timer(1.0 / 100);
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_keyboard_event_source());
    // al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));
    al_register_event_source(queue, al_get_audio_stream_event_source(audio_stream));
    al_start_timer(timer);
    return 0;
}

void destroy_allegro()
{
    al_destroy_event_queue(queue);
    al_destroy_timer(timer);
    al_destroy_audio_stream(audio_stream);
    al_uninstall_audio();
    al_destroy_display(display);
    al_uninstall_keyboard();
    free_midi_playback();
}

ALLEGRO_FONT *get_font()
{
    return font;
}
