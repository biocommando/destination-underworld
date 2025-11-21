#include "allegro_management.h"
#include "allegro5/allegro_acodec.h"
#include "allegro5/allegro_ttf.h"
#include "du_constants.h"
#include "gen_version_info.h"
#include "midi_playback.h"
#include "settings.h"
#include <stdio.h>

static char keybuffer[ALLEGRO_KEY_MAX];

static ALLEGRO_FONT *menu_font = NULL;
static ALLEGRO_FONT *menu_title_font = NULL;
static ALLEGRO_FONT *game_font = NULL;
static ALLEGRO_FONT *game_font_tiny = NULL;
static ALLEGRO_DISPLAY *display = NULL;
static ALLEGRO_TIMER *timer = NULL;
static ALLEGRO_EVENT_QUEUE *io_queue = NULL;
static ALLEGRO_EVENT_QUEUE *timer_queue = NULL;
static ALLEGRO_AUDIO_STREAM *audio_stream = NULL;

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

int check_keys(const int *keys, int num_keys)
{
    for (int i = 0; i < num_keys; i++)
    {
        if (check_key(keys[i]))
            return keys[i];
    }
    return 0;
}

int get_key()
{
    for (int i = 0; i < ALLEGRO_KEY_MAX; i++)
    {
        if (keybuffer[i])
            return i;
    }
    return 0;
}

// Counter for adding a second of silence (or delay tail) after every midi file
static int midi_track_spacing_counter = 0;

#define TIMER_MS 2

void wait_timer_event()
{
    ALLEGRO_EVENT event;
    ALLEGRO_TIMEOUT tmo;
    al_init_timeout(&tmo, 0.05);
    al_wait_for_event_until(timer_queue, &event, &tmo);
}

void consume_event_queue()
{
    ALLEGRO_EVENT event;

    while (al_get_next_event(io_queue, &event))
    {
        if (event.type == ALLEGRO_EVENT_KEY_DOWN)
        {
            keybuffer[event.keyboard.keycode] = 1;
        }
        else if (event.type == ALLEGRO_EVENT_KEY_UP)
        {
            keybuffer[event.keyboard.keycode] = 0;
        }
        else if (event.type == ALLEGRO_EVENT_AUDIO_STREAM_FRAGMENT)
        {
            if (!get_game_settings()->music_on)
                continue;
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
    }
}

void wait_delay(int v)
{
    while (v > 0)
    {
        wait_timer_event();
        consume_event_queue();
        v--;
    }
}

void wait_delay_ms(int ms)
{
    wait_delay(ms / TIMER_MS);
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
        consume_event_queue();
        for (int i = 0; i < num_keys; i++)
        {
            if (check_key(keys[i]))
            {
                key = keys[i];
            }
        }
        if (key == -1)
        {
            wait_timer_event();
        }
    }
    wait_key_release(key);
    return key;
}

void wait_key_release(int key)
{
    while (keybuffer[key])
    {
        consume_event_queue();
        wait_timer_event();
    }
}

int init_allegro()
{
#define NOT_NULL(ptr) (ptr) != NULL
#define IS_TRUE(thing) (thing) == 1
#define TRY_INIT(command, expectation)                            \
    if (!(expectation(command)))                                  \
    {                                                             \
        puts("ERROR: Init allegro failed at command: " #command); \
        return 1;                                                 \
    }

    TRY_INIT(al_init(), IS_TRUE)
    memset(keybuffer, 0, sizeof(keybuffer));
    // al_install_mouse();
    TRY_INIT(al_install_keyboard(), IS_TRUE);
    TRY_INIT(al_init_image_addon(), IS_TRUE);
    TRY_INIT(al_init_primitives_addon(), IS_TRUE);
    TRY_INIT(al_init_font_addon(), IS_TRUE);
    TRY_INIT(al_init_ttf_addon(), IS_TRUE);
    TRY_INIT(al_init_acodec_addon(), IS_TRUE);

    al_set_new_window_title("Destination Underworld " DU_VERSION);
    al_set_new_display_refresh_rate(60);
    int fullscreen_flag = get_game_settings()->fullscreen ? ALLEGRO_FULLSCREEN : 0;
    al_set_new_display_flags(ALLEGRO_OPENGL | fullscreen_flag);
    TRY_INIT(menu_font = al_load_ttf_font(get_game_settings()->menu_font, 16, ALLEGRO_TTF_NO_KERNING), NOT_NULL);
    TRY_INIT(menu_title_font = al_load_ttf_font(get_game_settings()->menu_font, 24, ALLEGRO_TTF_NO_KERNING), NOT_NULL);
    TRY_INIT(game_font = al_load_ttf_font(get_game_settings()->game_font, 12, ALLEGRO_TTF_NO_KERNING), NOT_NULL);
    TRY_INIT(game_font_tiny = al_load_ttf_font(get_game_settings()->game_font, 8, ALLEGRO_TTF_NO_KERNING | ALLEGRO_TTF_MONOCHROME), NOT_NULL);
    TRY_INIT(display = al_create_display(DISPLAY_W, DISPLAY_H), NOT_NULL);

    TRY_INIT(al_install_audio(), IS_TRUE);
    TRY_INIT(al_reserve_samples(64), IS_TRUE);

    TRY_INIT(audio_stream = al_create_audio_stream(4, AUDIO_BUFFER_SIZE, 44100, ALLEGRO_AUDIO_DEPTH_FLOAT32, ALLEGRO_CHANNEL_CONF_2), NOT_NULL);
    ALLEGRO_MIXER *mixer;
    TRY_INIT(mixer = al_get_default_mixer(), NOT_NULL);
    TRY_INIT(al_attach_audio_stream_to_mixer(audio_stream, mixer), IS_TRUE);

    init_midi_playback(44100);

    TRY_INIT(timer = al_create_timer(TIMER_MS / 1000.0), NOT_NULL);
    TRY_INIT(io_queue = al_create_event_queue(), NOT_NULL);
    TRY_INIT(timer_queue = al_create_event_queue(), NOT_NULL);
    al_register_event_source(io_queue, al_get_keyboard_event_source());
    // al_register_event_source(queue, al_get_mouse_event_source());
    // al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(timer_queue, al_get_timer_event_source(timer));
    al_register_event_source(io_queue, al_get_audio_stream_event_source(audio_stream));
    al_start_timer(timer);
    if (al_get_display_refresh_rate(display) < 50)
    {
        printf("NOTE! Display rate %d Hz discovered. Game is optimized for 50 Hz or above.\n"
               "Game will run but with a slower speed\n",
               al_get_display_refresh_rate(display));
    }
    return 0;
}

void destroy_allegro()
{
    al_destroy_event_queue(io_queue);
    al_destroy_event_queue(timer_queue);
    al_destroy_timer(timer);
    al_destroy_audio_stream(audio_stream);
    al_uninstall_audio();
    al_destroy_display(display);
    al_uninstall_keyboard();
    free_midi_playback();
    al_destroy_font(menu_font);
    al_destroy_font(game_font);
    al_destroy_font(game_font_tiny);
}

ALLEGRO_FONT *get_font()
{
    return game_font;
}

ALLEGRO_FONT *get_font_tiny()
{
    return game_font_tiny;
}

ALLEGRO_FONT *get_menu_font()
{
    return menu_font;
}

ALLEGRO_FONT *get_menu_title_font()
{
    return menu_title_font;
}

ALLEGRO_BITMAP *get_screen()
{
    return al_clone_bitmap(al_get_backbuffer(display));
}
