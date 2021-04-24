#ifndef SAMPLEREGISTER_H
#define SAMPLEREGISTER_H
#include "allegro.h"

void registerSample(int id, const char *fname);

void triggerSample(int id, int volume);
void triggerSampleWithParams(int id, int volume, int pan, int pitch);

void resetSampleTriggers();

void destroyRegisteredSamples();

#endif
