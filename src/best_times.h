#pragma once

#define NUM_BEST_TIMES 10

struct best_times
{
    int game_modifiers;
    int mission;
    float times[NUM_BEST_TIMES];
};

int populate_best_times(const char *mission_pack, struct best_times *best_times);

int save_best_times(const char *mission_pack, struct best_times *best_times);

int check_time_beaten(struct best_times *best_times, float time);