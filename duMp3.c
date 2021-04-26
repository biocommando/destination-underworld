#include <stdio.h>
#include <stdlib.h>
#include "allegro.h"
#include "dump3.h"

int music_on = 1;
MP3FILE *mp3 = NULL;

void next_track()
{
  static int current_track = 0;

  char filename[32];
  current_track = current_track < 3 ? current_track + 1 : 1;
  sprintf(filename, ".\\dataloss\\music%d.mp3", current_track);
  close_mp3_file(mp3);
  mp3 = open_mp3_file(filename);
}

void play_mp3()
{
  static int music_on_status = 0;
  if (music_on_status != music_on)
  {
    if (music_on)
      play_mp3_file(mp3, BUFSZ, 255, 127);
    else
    {
      next_track();
    }
  }
  if (music_on)
  {
    if (((mp3) && (poll_mp3_file(mp3) != ALMP3_OK)))
    {
      next_track();
      play_mp3_file(mp3, BUFSZ, 255, 127);
    }
  }
  music_on_status = music_on;
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
