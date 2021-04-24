#include "sampleRegister.h"

int sampleRegIdx = 0;

struct {
         int id;
         int triggered;
         SAMPLE *sample;
       } sampleRegister[64];

void registerSample(int id, const char *filename)
{
     if (sampleRegIdx < 64)
     {
       sampleRegister[sampleRegIdx].id = id;
       sampleRegister[sampleRegIdx].sample = load_sample(filename);
       //printf("Registered sample %d %d %s\n", sampleRegIdx, id, filename);
       sampleRegIdx++;
     }
}

void triggerSampleWithParams(int id, int volume, int pan, int pitch)
{
 for (int i = sampleRegIdx - 1; i >= 0; i--)
 {
  if (sampleRegister[i].id == id)
  {
    if (!sampleRegister[i].triggered) play_sample(sampleRegister[i].sample, volume, pan, pitch, 0);
    sampleRegister[i].triggered = 0;
    return;
  }
 }
}

void triggerSample(int id, int volume)
{
 triggerSampleWithParams(id, volume, 127, 800 + rand() % 400);
}

void resetSampleTriggers()
{
 for (int i = sampleRegIdx - 1; i >= 0; i--) sampleRegister[i].triggered = 0;
}

void destroyRegisteredSamples()
{
     for (int i = sampleRegIdx - 1; i >= 0; i--) destroy_sample(sampleRegister[i].sample);
}
