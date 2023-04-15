#pragma once
#include "allegro42_compat.h"


void switch_track(int track_number);
int get_current_track();
void play_mp3();

void preload_mp3s();
void destroy_mp3s();
