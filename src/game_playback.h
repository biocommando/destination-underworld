#pragma once

void game_playback_init(const char *fn, char mode);

long game_playback_get_key_mask();

// Get the timestamp for the event.
// If there's no event, return -1.
long game_playback_get_time_stamp();

void game_playback_add_key_event(long time_stamp, long key_mask);
void game_playback_add_end_event();

void game_playback_next();