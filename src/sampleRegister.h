#pragma once

#include "allegro42_compat.h"

#define MAX_NUM_SAMPLES 64

// TODO: the explanation is no longer valid as music does not use the same sample
// playback interface
// Default priority = 128.
// To eliminate the need to do some hacks to keep the music playing
// let's keep the priority numbers below that
#define SAMPLE_PRIORITY_NORMAL 40
#define SAMPLE_PRIORITY_HIGH 80
#define SAMPLE_PRIORITY_LOW 0

#define SAMPLE_PRIORITY(base_level, add_level) (SAMPLE_PRIORITY_##base_level + add_level)

/*
 * Set sample with the given filename to sample registry with the given id and priority.
 */
void register_sample(int id, const char *fname, int priority);

/*
 * Same as trigger_sample_with_params but only volume can be altered.
 * Uses center panning and randomizes pitch a bit for variation.
 */
void trigger_sample(int id, int volume);

/*
 * Trigger sample with given id using specific volume, panning and pitch.
 * Plays samnple only if it's triggered the first time after resetting triggers.
 * If playing fails, will try to stop a lower priority sample and try again.
 */
void trigger_sample_with_params(int id, int volume, int pan, int pitch);

/*
 * Resets the sample triggers so samples with certain id can be played again.
 */
void reset_sample_triggers();

/*
 * Free all samples in the sample regiester.
 */
void destroy_registered_samples();

/*
 * Stop all samples in the register from playing.
 */
void stop_all_samples();
