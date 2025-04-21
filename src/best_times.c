#include "best_times.h"

#include "record_file.h"
#include "duConstants.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BEST_TIMES_FILE DATADIR "%s/best_times.dat"

static int compare_floats(const void *a, const void *b)
{
    float arg1 = *(const float *)a;
    float arg2 = *(const float *)b;

    if (arg1 < arg2)
        return -1;
    if (arg1 > arg2)
        return 1;
    return 0;
}

static inline void get_id(char *id, struct best_times *best_times, int idx)
{
    sprintf(id, "MISSION=%d;MODE=%d;I=%d;", best_times->mission, best_times->game_modifiers, idx);
}

static inline void get_file(char *file, const char *mission_pack)
{
    sprintf(file, BEST_TIMES_FILE, mission_pack);
}

int populate_best_times(const char *mission_pack, struct best_times *best_times)
{
    memset(best_times->times, 0, sizeof(best_times->times));

    char file[1024];
    get_file(file, mission_pack);

    char id[100];
    for (int i = 0; i < NUM_BEST_TIMES; i++)
    {
        get_id(id, best_times, i);
        best_times->times[i] = 1e10;
        record_file_scanf(file, id, "%*s %f", &best_times->times[i]);
    }

    qsort(best_times->times, NUM_BEST_TIMES, sizeof(float), compare_floats);

    return 0;
}

int save_best_times(const char *mission_pack, struct best_times *best_times)
{
    char file[1024];
    get_file(file, mission_pack);

    char id[100];
    for (int i = 0; i < NUM_BEST_TIMES; i++)
    {
        get_id(id, best_times, i);
        if (record_file_set_record_f(file, "%s %f", id, best_times->times[i]))
        {
            return 1;
        }
    }
    return 0;
}

int check_time_beaten(struct best_times *best_times, float time)
{
    int idx_beaten = NUM_BEST_TIMES;
    for (int i = 0; i < NUM_BEST_TIMES; i++)
    {
        if (time < best_times->times[i])
        {
            idx_beaten = i;
            break;
        }
    }
    if (idx_beaten == NUM_BEST_TIMES)
        return -1;

    for (int i = NUM_BEST_TIMES - 1; i > idx_beaten; i--)
    {
        best_times->times[i] = best_times->times[i - 1];
    }

    best_times->times[idx_beaten] = time;

    return idx_beaten;
}