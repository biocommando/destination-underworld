#pragma once

#define MAX_NUM_MP3S 32

/**
 * @brief Preload a number of music files into memory.
 * 
 * @param path Path to the file.
 * @return Index of the file or -1 if failed.
 */
int preload_music_track(const char *path);

/**
 * @brief Destroy all music tracks and free memory.
 * 
 */
void destroy_music_tracks();

/**
 * @brief Play file at index. If index changes, the file will be stopped and
 * the new one will be played.
 * 
 * @return 0 if track is not at end, 1 if track is at end. -1 if failed.
 */
int music_track_play(int index);

/**
 * @brief Stop the current track.
 * 
 */
void music_track_stop();
