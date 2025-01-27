#pragma once

/*
 * Show a screen that indicates that the game is processing something that takes some time.
 * The status argument is the text shown and if the display_load_state is 1, the screen
 * contains also a progress bar.
 */
void progress_load_state(const char *status, int display_load_state);