#include "musicTrack.h"

#include "allegro5/allegro.h"
#include "allegro5/allegro_audio.h"
struct MusicTrackState
{
    ALLEGRO_SAMPLE *sample;
    ALLEGRO_SAMPLE *samples[MAX_NUM_MP3S];
    ALLEGRO_SAMPLE_INSTANCE *sample_instance;
    int sample_count;
    int current_music_track;
};

struct MusicTrackState *get_music_track_state()
{
    static struct MusicTrackState music_track_state;
    static int init_state = 0;
    if (!init_state)
    {
        init_state = 1;
        memset(&music_track_state, 0, sizeof(struct MusicTrackState));
        music_track_state.current_music_track = -1;
    }
    return &music_track_state;
}

int preload_music_track(const char *path)
{
    struct MusicTrackState *state = get_music_track_state();
    if (state->sample_count >= MAX_NUM_MP3S)
        return -1;

    state->samples[state->sample_count] = al_load_sample(path);
    if (!state->samples[state->sample_count])
    {
        return -1;
    }
    state->sample_count++;
    return state->sample_count - 1;
}

void deinit_sample_instance()
{
    struct MusicTrackState *state = get_music_track_state();
    if (state->sample_instance)
    {
        al_destroy_sample_instance(state->sample_instance);
        state->sample_instance = NULL;
    }
}

int music_track_play(int idx)
{
    struct MusicTrackState *state = get_music_track_state();
    if (idx < 0 || idx >= state->sample_count)
        return -1;
    if (idx != state->current_music_track)
    {
        deinit_sample_instance();
        state->sample = state->samples[idx];
        state->sample_instance = al_create_sample_instance(state->sample);
        al_attach_sample_instance_to_mixer(state->sample_instance, al_get_default_mixer());
        al_play_sample_instance(state->sample_instance);
        state->current_music_track = idx;
    }
    return al_get_sample_instance_playing(state->sample_instance) ? 0 : 1;
}

void music_track_stop()
{
    struct MusicTrackState *state = get_music_track_state();
    if (state->sample_instance)
    {
        al_stop_sample_instance(state->sample_instance);
        deinit_sample_instance();
        state->current_music_track = -1;
    }
}

void destroy_music_tracks()
{
    deinit_sample_instance();
    struct MusicTrackState *state = get_music_track_state();
    for (int i = 0; i < state->sample_count; i++)
    {
        al_destroy_sample(state->samples[i]);
    }
    state->sample_count = 0;
}