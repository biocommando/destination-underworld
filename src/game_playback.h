#pragma once

/*
 * Initialize game playback (= "demo mode"). The key inputs will be saved to/loaded from
 * the file provided with game_playback_set_filename.
 *
 * The file uses record file format with the following syntax:
 * key:
 * event_[index]
 * parameters:
 * - end
 * - time
 * - keys
 *
 * Note that the key values are just for human-readability, the key-value order needs to be exactly like this.
 * The keys have the following meaning:
 * - end: 1 = no more events, 0 = has more events
 * - time: the frame index at which the keymask becomes active
 * - keys: the key mask containing all the control keys (4 direction keys, 2 weapon select keys,
 *         4 power up keys, shoot key = 11 bits).
 *         Bit 11 is used to notify that the active perks have changed.
 *         If this bit is set, the 16 uppermost bits are used for saving the new perks mask.
 */
void game_playback_init();

/*
 * Sets filename to use for recording / playback. Max. 255 characters
 */
void game_playback_set_filename(const char *fn);

/*
 * Gets filename to use for recording / playback. Max. 255 characters
 */
const char *game_playback_get_filename();

/*
 * Get the key mask at the current playback position.
 */
long game_playback_get_key_mask();

// Get the timestamp for the current event.
// If there's no more events (=this is the "end event"), return -1.
long game_playback_get_time_stamp();

/*
 * Set a keymask event to the current playback index in write mode.
 */
void game_playback_add_key_event(long time_stamp, long key_mask);

/*
 * Set a keymask event to the current playback index in write mode.
 */
void game_playback_add_end_event();

/*
 * Increment the game playback event index.
 */
void game_playback_next();

/*
 * Get pointer to the current playback mode (none, play, record).
 */
int *get_playback_mode();