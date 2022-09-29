#ifndef DUMP3_H
#define DUMP3_H
#include "allegro42_compat.h"

#define DATASZ (1 << 15) /* (32768) amount of data to read from disk each time */
#define BUFSZ (1 << 16)  /* (65536) size of audiostream buffer */
typedef int PACKFILE;
typedef int ALMP3_MP3STREAM;
typedef struct
{
  PACKFILE *f;
  ALMP3_MP3STREAM *s;
} MP3FILE;

void open_mp3_file(char *filename);
void close_mp3_file();
int play_mp3_file(MP3FILE *mp3, int buflen, int vol, int pan);
void play_track(int track_number);
int get_current_track();
void play_mp3();

void play_mp3_in_thread();

#endif
