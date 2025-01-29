#include "sampleRegister.h"
#include "settings.h"
#include "duConstants.h"
#include "logging.h"
#include "record_file.h"
#include <stdio.h>

extern GameSettings game_settings;

static int sample_reg_idx = 0;

struct
{
  int id;
  int triggered;
  ALLEGRO_SAMPLE *sample;
  int priority;
  ALLEGRO_SAMPLE_ID sample_id;
} sample_register[MAX_NUM_SAMPLES];

static int get_sample_path(char *sample_path, const char *sample_name)
{
  char sample_mappings_filename[256];
  get_data_filename(sample_mappings_filename, "sounds.dat");
  char record[256] = "", value[256] = "";
  if (record_file_get_record(sample_mappings_filename, sample_name, record, sizeof(record)) == 0 &&
      sscanf(record, "%*s %s", value) == 1)
  {
    get_data_filename(sample_path, value);
    return 0;
  }
  return 1;
}

void register_sample(int id, const char *sample_name, int priority)
{
  if (sample_reg_idx < MAX_NUM_SAMPLES)
  {
    char sample_path[256] = "";
    if (get_sample_path(sample_path, sample_name) != 0)
    {
      printf("ERROR: Sample mapping not found for \"%s\"\n", sample_name);
      return;
    }
    sample_register[sample_reg_idx].id = id;

    ALLEGRO_SAMPLE *sample = al_load_sample(sample_path);
    if (!sample)
    {
      printf("ERROR: Loading sample \"%s\" from path \"%s\" failed\n", sample_name, sample_path);
      return;
    }
    sample_register[sample_reg_idx].sample = sample;
    sample_register[sample_reg_idx].priority = priority;
    memset(&sample_register[sample_reg_idx].sample_id, 0, sizeof(ALLEGRO_SAMPLE_ID));
    sample_reg_idx++;
  }
}

void stop_all_samples()
{
  for (int i = sample_reg_idx - 1; i >= 0; i--)
  {
    al_stop_sample(&sample_register[i].sample_id);
  }
}

void trigger_sample_with_params(int id, int volume, int pan, int pitch)
{
  const ALLEGRO_PLAYMODE pm = ALLEGRO_PLAYMODE_ONCE;
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
        if (!al_play_sample(sample_register[i].sample, gain, normpan, normpitch, pm, &sample_register[i].sample_id))
        {
          for (int j = sample_reg_idx - 1; j >= 0; j--)
          {
            if (sample_register[j].priority < sample_register[i].priority)
              al_stop_sample(&sample_register[j].sample_id);
          }
          al_play_sample(sample_register[i].sample, gain, normpan, normpitch, pm, &sample_register[i].sample_id);
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
    al_destroy_sample(sample_register[i].sample);
  }
}
