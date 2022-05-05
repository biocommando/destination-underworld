#ifndef SAMPLEREGISTER_H
#define SAMPLEREGISTER_H
#include "allegro.h"

void register_sample(int id, const char *fname);

void trigger_sample(int id, int volume);
void trigger_sample_with_params(int id, int volume, int pan, int pitch);

void reset_sample_triggers();

void destroy_registered_samples();

#endif
