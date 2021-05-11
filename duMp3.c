#include <stdio.h>
#include <stdlib.h>
#include "allegro.h"
#include "dump3.h"
#include "settings.h"

extern GameSettings game_settings;

MP3FILE *mp3 = NULL;

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
  sprintf(filename, ".\\dataloss\\music%d.mp3", current_track);
  close_mp3_file(mp3);
  mp3 = open_mp3_file(filename);
  play_mp3_file(mp3, BUFSZ, 255, 127);
}

void play_mp3()
{
  static int music_on_status = 0;
  if (music_on_status != game_settings.music_on)
  {
    if (!game_settings.music_on)
      almp3_stop_mp3stream(mp3->s);
    else
      play_mp3_file(mp3, BUFSZ, 255, 127);
  }
  if (game_settings.music_on)
  {
    if (mp3 && poll_mp3_file(mp3) != ALMP3_OK)
    {
      play_track(current_track + 1);
      play_mp3_file(mp3, BUFSZ, 255, 127);
    }
  }
  music_on_status = game_settings.music_on;
}

/****************MP3MP3MP3MP3**********************/

/*MP3-rutiinit*/

MP3FILE *open_mp3_file(char *filename)
{
  MP3FILE *p = NULL;
  PACKFILE *f = NULL;
  ALMP3_MP3STREAM *s = NULL;
  char data[DATASZ];
  int len;

  if (!(p = (MP3FILE *)malloc(sizeof(MP3FILE))))
    goto error;
  if (!(f = pack_fopen(filename, F_READ)))
    goto error;
  if ((len = pack_fread(data, DATASZ, f)) <= 0)
    goto error;
  if (len < DATASZ)
  {
    if (!(s = almp3_create_mp3stream(data, len, TRUE)))
      goto error;
  }
  else
  {
    if (!(s = almp3_create_mp3stream(data, DATASZ, FALSE)))
      goto error;
  }
  p->f = f;
  p->s = s;
  return p;

error:
  pack_fclose(f);
  free(p);
  return NULL;
}

int play_mp3_file(MP3FILE *mp3, int buflen, int vol, int pan)
{
  return almp3_play_mp3stream(mp3->s, buflen, vol, pan);
}

void close_mp3_file(MP3FILE *mp3)
{
  if (mp3)
  {
    pack_fclose(mp3->f);
    almp3_destroy_mp3stream(mp3->s);
    free(mp3);
    mp3 = NULL;
  }
}

int poll_mp3_file(MP3FILE *mp3)
{
  char *data;
  long len;

  data = (char *)almp3_get_mp3stream_buffer(mp3->s);
  if (data)
  {
    len = pack_fread(data, DATASZ, mp3->f);
    if (len < DATASZ)
      almp3_free_mp3stream_buffer(mp3->s, len);
    else
      almp3_free_mp3stream_buffer(mp3->s, -1);
  }

  return almp3_poll_mp3stream(mp3->s);
}

/****************MP3MP3MP3MP3MP3*******************/
