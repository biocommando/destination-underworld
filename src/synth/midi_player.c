#include "midi_player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_midi_player(MidiPlayer *p, float sample_rate)
{
    p->sample_rate = sample_rate;
    init_Synth(&p->synth, sample_rate);
    p->midi_file = NULL;
    p->next_track_event_at = NULL;
    p->next_track_event_idx = NULL;

    p->tempo = 120;
    p->sample_rate;
    p->pos = 0;
    p->tick_pos = 0;
    p->tracks_at_end = 0;
    p->loop = 0;
    p->ended = 0;
}

void midi_player_init_track_event_lists(MidiPlayer *p, MidiFile *mf)
{
    for (int trk = 0; trk < mf->track_count; trk++)
    {
        p->next_track_event_at[trk] = mf->tracks[trk].events[0].time_delta;
        p->next_track_event_idx[trk] = 0;
    }
    p->pos = 0;
    p->tracks_at_end = 0;
}

void free_midi_player(MidiPlayer *p)
{
    free(p->next_track_event_at);
    free(p->next_track_event_idx);
    free_Synth(&p->synth);
}

void midi_player_set_midi_file(MidiPlayer *p, MidiFile *midi_file)
{
    p->midi_file = midi_file;
    free(p->next_track_event_at);
    free(p->next_track_event_idx);
    p->next_track_event_at = (unsigned *)malloc(sizeof(unsigned) * midi_file->track_count);
    p->next_track_event_idx = (unsigned *)malloc(sizeof(unsigned) * midi_file->track_count);

    p->pos = 0;
    p->tick_pos = 0;
    p->tracks_at_end = 0;
    p->ended = 0;

    p->tempo = midi_file->tempo;

    midi_player_init_track_event_lists(p, midi_file);

    // If track has not ended, all existing voices will sound forever
    Synth_kill_all_voices(&p->synth);
}

void midi_player_process_buffer(MidiPlayer *p, float *buf, int size)
{
    if (!p->midi_file)
        return;
    size *= 2; // stereo
    if (p->ended)
    {
        // Don't cut off e.g. delay tail
        Synth_process(&p->synth, buf, NULL, size);
        return;
    }
    int process_at_idx = 0;
    // in stereo audio every other sample is for left and every other for right
    // so let's make sure that we run any logic only when index is at left channel
    // so we don't need to care about misalignment of the channels
    for (int i = 0; i < size; i += 2)
    {
        if (++p->tick_pos == p->midi_file->samples_per_tick)
        {
            Synth_process(&p->synth, &buf[process_at_idx], NULL, i - process_at_idx);

            process_at_idx = i;
            p->tick_pos = 0;
            for (int trk = 0; trk < p->midi_file->track_count; trk++)
            {
                MidiEvent *events = p->midi_file->tracks[trk].events;
                while (p->pos == p->next_track_event_at[trk])
                {
                    /*std::cout << "Handling midi event\n";
                    auto &evt = events[next_track_event_idx[trk]];
                    std::cout << ((evt.data[0] & 0xF0) == 0b10000000 ? "note off" : "note on") << ", key="
                        << std::to_string(evt.data[1]) << ", vel=" << std::to_string(evt.data[2])
                        << ". TD=" << std::to_string(evt.time_delta) << ", instrument:" << std::to_string(evt.data[0]&0xF) << "\n";*/
                    if (!events[p->next_track_event_idx[trk]].end_of_track)
                        Synth_handle_midi_event(&p->synth, events[p->next_track_event_idx[trk]].data, 0);
                    p->next_track_event_idx[trk]++;
                    if (p->next_track_event_idx[trk] < p->midi_file->tracks[trk].count)
                    {
                        p->next_track_event_at[trk] = p->pos + events[p->next_track_event_idx[trk]].time_delta;
                    }
                    else
                    {
                        p->next_track_event_at[trk] = ~0;
                        p->tracks_at_end++;
                    }
                }
            }
            p->pos++;
            if (p->tracks_at_end == p->midi_file->track_count)
            {
                if (p->loop)
                    midi_player_init_track_event_lists(p, p->midi_file);
                else
                    p->ended = 1;
            }
        }
    }
    Synth_process(&p->synth, &buf[process_at_idx], NULL, size - process_at_idx);
}