#include <stdio.h>
#include <stdlib.h>
#include "allegro42_compat.h"
#include "dump3.h"
#include "musicTrack.h"
#include "settings.h"
#include "duConstants.h"
#include "loadindicator.h"

extern GameSettings game_settings;

int current_track = 1;

int get_current_track()
{
  return current_track;
}

void switch_track(int track_number)
{
  while (track_number <= 0)
    track_number += game_settings.num_music_tracks;
  while (track_number > game_settings.num_music_tracks)
    track_number -= game_settings.num_music_tracks;

  current_track = track_number;
}

void play_mp3()
{
  static int music_on_status = -1;
  if (music_on_status != game_settings.music_on)
  {
    if (!game_settings.music_on)
    {
      music_track_stop();
    }
    else
      switch_track(current_track);
  }
  if (game_settings.music_on)
  {
    if (music_track_play(current_track - 1) == 1)
    {
      switch_track(current_track + 1);
    }
  }
  music_on_status = game_settings.music_on;
}

void preload_mp3s()
{
  int i;
  char filename[32];
  for (i = 1; i <= game_settings.num_music_tracks; i++)
  {
    sprintf(filename, DATADIR "music%d.mp3", i);
    progress_load_state(filename, 1);
    if (preload_music_track(filename) < 0)
    {
      printf("Error loading music track %d\n", i);
    }
  }
}

void destroy_mp3s()
{
  destroy_music_tracks();
}
