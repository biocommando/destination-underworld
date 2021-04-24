#ifndef DUMP3_H
#define DUMP3_H
#include "almp3.h"

#define DATASZ  (1<<15) /* (32768) amount of data to read from disk each time */
#define BUFSZ   (1<<16) /* (65536) size of audiostream buffer */

typedef struct {
  PACKFILE *f;
  ALMP3_MP3STREAM *s;
} MP3FILE;

MP3FILE *open_mp3_file(char *filename);
int poll_mp3_file(MP3FILE *mp3);
void close_mp3_file(MP3FILE *mp3);
int play_mp3_file(MP3FILE *mp3, int buflen, int vol, int pan);
void nextTrack();
void playMP3();

void playMp3InThread();
        
    
#endif
