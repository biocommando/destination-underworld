#pragma once

#include "midi_reader.h"
#include "synth.h"

// Ported from C++ code

/*
 * Midi player that uses the Synth syntheziser as the sound engine.
 * Each synth instrument is selected by the midi channel.
 * Only supports note on/off events and tempo header information.
 */
typedef struct
{
    // Tempo in bpm
    float tempo; // default = 120;
    // Sample rate in Hz
    float sample_rate;
    // Sequencer position in midi ticks
    unsigned pos; // default = 0;
    // Sample position within one midi tick
    int tick_pos; // default = 0;
    // 1 if all tracks (=midi channels) have ended
    int tracks_at_end; // default = 0;

    // The midi file used as the sequencer input data
    MidiFile *midi_file;

    // Array containing the next event midi tick position for each track.
    // For tracks at end, the value is set to unsigned int max value.
    unsigned *next_track_event_at;
    // The next track event index
    unsigned *next_track_event_idx;

    // Synth engine. Note that setting instrument parameters should be done by the caller.
    Synth synth;

    // Flag for deciding if the sequencer should loop to start or stay
    int loop;
    // Flag to determine that the sequencer has reached the end. Never 1 when loop = 1.
    int ended;
} MidiPlayer;

/*
 * Initializes the midi player and the synth engine using the provided sample rate (Hz).
 */
void init_midi_player(MidiPlayer *p, float sample_rate);

/*
 * Frees all memory associated with the structure.
 */
void free_midi_player(MidiPlayer *p);

/*
 * Sets a new midi file to the sequencer. Can be called multiple times between init and free.
 */
void midi_player_set_midi_file(MidiPlayer *p, MidiFile *midi_file);

/*
 * Processes a stereo sample buffer (samples are expected to be aligned as L, R, L, R, ...)
 * with size stereo samples and proceeds the state of the sequencer.
 */
void midi_player_process_buffer(MidiPlayer *p, float *buf, int size);
