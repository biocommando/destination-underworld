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
  int priority;
  ALLEGRO_SAMPLE_ID sample_id;
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
    sample_register[sample_reg_idx].priority = priority;
    memset(&sample_register[sample_reg_idx].sample_id, 0, sizeof(ALLEGRO_SAMPLE_ID));
    sample_reg_idx++;
  }
}

void stop_all_samples()
{
  //al_stop_samples();
  for (int i = sample_reg_idx - 1; i >= 0; i--)
  {
    al_stop_sample(&sample_register[i].sample_id);
  }
}

void trigger_sample_with_params(int id, int volume, int pan, int pitch)
{
  if (game_settings.sfx_vol == 0)
    return;
  double gain = volume / 255.0 * game_settings.sfx_vol;
  double normpan = (pan - 127) / 128.0;
  double normpitch = pitch / 1000.0;
  for (int i = sample_reg_idx - 1; i >= 0; i--)
  {
    if (sample_register[i].id == id)
    {
      if (!sample_register[i].triggered)
      {
        if (!play_sample(sample_register[i].sample, gain, normpan, normpitch, 0, &sample_register[i].sample_id))
        {
          for (int j = sample_reg_idx - 1; j >= 0; j--)
          {
            if (sample_register[j].priority < sample_register[i].priority)
              al_stop_sample(&sample_register[j].sample_id);
          }
          play_sample(sample_register[i].sample, gain, normpan, normpitch, 0, &sample_register[i].sample_id);
        }
      }
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
