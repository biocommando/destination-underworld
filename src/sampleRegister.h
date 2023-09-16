#pragma once

#include "allegro42_compat.h"

#define MAX_NUM_SAMPLES 64

// Default priority = 128.
// To eliminate the need to do some hacks to keep the music playing
// let's keep the priority numbers below that
#define SAMPLE_PRIORITY_NORMAL 40
#define SAMPLE_PRIORITY_HIGH 80
#define SAMPLE_PRIORITY_LOW 0

#define SAMPLE_PRIORITY(base_level, add_level) (SAMPLE_PRIORITY_##base_level + add_level)

void register_sample(int id, const char *fname, int priority);

void trigger_sample(int id, int volume);
void trigger_sample_with_params(int id, int volume, int pan, int pitch);

void reset_sample_triggers();

void destroy_registered_samples();

void stop_all_samples();
