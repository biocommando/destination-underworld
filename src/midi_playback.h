#pragma once

#include "synth/midi_player.h"

void init_midi_playback(float sample_rate);
void free_midi_playback();

MidiPlayer *get_midi_player();

void next_midi_track();

void randomize_midi_playlist();
