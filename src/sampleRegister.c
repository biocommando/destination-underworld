#include "sampleRegister.h"
#include "settings.h"
#include "duConstants.h"
#include "logging.h"
#include <stdio.h>

extern GameSettings game_settings;

int sample_reg_idx = 0;

struct
{
  int id;
  int triggered;
  SAMPLE *sample;
} sample_register[MAX_NUM_SAMPLES];

void register_sample(int id, const char *filename, int priority)
{
  if (sample_reg_idx < MAX_NUM_SAMPLES)
  {
    sample_register[sample_reg_idx].id = id;
    char sample_path[256];
    if (game_settings.custom_resources)
    {
      sprintf(sample_path, DATADIR "%s\\%s", game_settings.mission_pack, filename);
    }
    else
    {
      sprintf(sample_path, DATADIR "%s", filename);
    }
    SAMPLE *sample = load_sample(sample_path);
    sample_register[sample_reg_idx].sample = sample;
    sample->priority = priority;
    sample_reg_idx++;
  }
}

void stop_all_samples()
{
  for (int i = sample_reg_idx - 1; i >= 0; i--)
  {
    stop_sample(sample_register[i].sample);
  }
}

void trigger_sample_with_params(int id, int volume, int pan, int pitch)
{
  for (int i = sample_reg_idx - 1; i >= 0; i--)
  {
    if (sample_register[i].id == id)
    {
      if (!sample_register[i].triggered)
        play_sample(sample_register[i].sample, volume * game_settings.sfx_vol, pan, pitch, 0);
      sample_register[i].triggered = 1;
      return;
    }
  }
}

void trigger_sample(int id, int volume)
{
  trigger_sample_with_params(id, volume, 127, 800 + rand() % 400);
}

void reset_sample_triggers()
{
  for (int i = sample_reg_idx - 1; i >= 0; i--)
    sample_register[i].triggered = 0;
}

void destroy_registered_samples()
{
  for (int i = sample_reg_idx - 1; i >= 0; i--)
  {
    destroy_sample(sample_register[i].sample);
  }
}
