#pragma once

#include "synth/midi_player.h"

/*
 * Initializes the midi playback engine. Needs to be called only once during the program.
 * Sample rate in Hz.
 *
 * Calling this function reads the playlist from datadir\midi-music by adding all the .mid files
 * that also have the metadata file that is suffixed with "_meta.ini" and preloads all .mid files.
 */
void init_midi_playback(float sample_rate);

/*
 * Frees all memory (midi player, midi file, synth etc.) associated to the midi playback.
 */
void free_midi_playback();

/*
 * Gets the internal MidiPlayer instance.
 */
MidiPlayer *get_midi_player();

/*
 * Sets the track to the requested index. -1 = next track in playlist (loops to start).
 */
void next_midi_track(int index);

/*
 * Randomize the playorder.
 */
void randomize_midi_playlist();

/*
 * Get the .mid file name without preceeding path at the playlist index. If there are no
 * tracks at the index, returns NULL.
 */
const char *get_midi_playlist_entry_file_name(int index);
