#pragma once

#define MAX_WT_SAMPLE_SLOTS 8

// One wavetable sample slot
struct wt_sample
{
    float *data;
    int size;
};

/*
 * Reads all wt_sample_slot_X.wav files from the directory.
 * Returns number of files read successfully.
 * All slots are initialized with a buffer of length 1 containing 0 so that
 * the caller doesn't need to do any null checking.
 * The read wavetable samples can be read using get_wt_sample.
 */
int wt_sample_read_all(const char *directory);

/*
 * Frees all the loaded samples.
 */
void wt_sample_free();

/*
 * Get the wavetable sample at given slot index. All slots should
 * contain valid data after wt_sample_read_all although not every
 * slot has a sample loaded. Slots without sample will be silent.
 */
struct wt_sample *get_wt_sample(unsigned slot);