#include <stdio.h>
#include <stdlib.h>
#include "allegro42_compat.h"
#include "dump3.h"
#include "settings.h"
#include "duConstants.h"

extern GameSettings game_settings;

ALLEGRO_SAMPLE *sample = NULL;
ALLEGRO_SAMPLE_INSTANCE *sample_instance = NULL;

int current_track = 1;

int get_current_track()
{
  return current_track;
}

void play_track(int track_number)
{
  while (track_number <= 0)
    track_number += game_settings.num_music_tracks;
  while (track_number > game_settings.num_music_tracks)
    track_number -= game_settings.num_music_tracks;

  current_track = track_number;

  char filename[32];
  sprintf(filename, DATADIR "music%d.mp3", current_track);
  open_mp3_file(filename);
}

void play_mp3()
{
  static int music_on_status = -1;
  if (music_on_status != game_settings.music_on)
  {
    if (!game_settings.music_on)
    {
      if (sample_instance)
        al_stop_sample_instance(sample_instance);
    }
    else
      play_track(current_track);
  }
  if (game_settings.music_on)
  {
    if (sample_instance && !al_get_sample_instance_playing(sample_instance))
    {
      play_track(current_track + 1);
    }
  }
  music_on_status = game_settings.music_on;
}

/****************MP3MP3MP3MP3**********************/

/*MP3-rutiinit*/

void open_mp3_file(char *filename)
{
  close_mp3_file();
  sample = al_load_sample(filename);
  sample_instance = al_create_sample_instance(sample);
  al_attach_sample_instance_to_mixer(sample_instance, al_get_default_mixer());
  al_play_sample_instance(sample_instance);
}

void close_mp3_file()
{
  if (sample)
  {
    /*pack_fclose(mp3->f);
    almp3_destroy_mp3stream(mp3->s);
    free(mp3);*/
    al_destroy_sample(sample);
    al_destroy_sample_instance(sample_instance);
    sample = NULL;
    sample_instance = NULL;
  }
}

/****************MP3MP3MP3MP3MP3*******************/
