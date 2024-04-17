#include "best_times.h"

#include "record_file.h"
#include "duConstants.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BEST_TIMES_FILE DATADIR "%s\\best_times.dat"

int _compare_floats(const void* a, const void* b)
{
    float arg1 = *(const float*)a;
    float arg2 = *(const float*)b;

    if (arg1 < arg2) return -1;
    if (arg1 > arg2) return 1;
    return 0;
}

void _get_id(char *id, struct best_times *best_times)
{
    sprintf(id, "MISSION=%d;MODE=%d;", best_times->mission, best_times->game_modifiers);
}

void _get_file(char *file, const char *mission_pack)
{
    sprintf(file, BEST_TIMES_FILE, mission_pack);
}

int populate_best_times(const char *mission_pack, struct best_times *best_times)
{
    memset(best_times->times, 0, sizeof(best_times->times));
    char record[1024];

    char file[1024];
    _get_file(file, mission_pack);

    char id[100];
    _get_id(id, best_times);

    if (record_file_get_record(file, id, record, sizeof(record)) == 0)
    {
        char *p = record;
        int i = -1;
        while (i < NUM_BEST_TIMES)
        {
            char *p2 = strstr(p, " ");
            if (!p2)
                break;
            *p2 = 0;
            if (i >= 0)
                best_times->times[i] = atof(p);
            p = p2 + 1;
            i++;
        }
        qsort(best_times->times, NUM_BEST_TIMES, sizeof(float), _compare_floats);
        return i == NUM_BEST_TIMES ? 0 : 1;
    }
    return 1;
}

int save_best_times(const char *mission_pack, struct best_times *best_times)
{
    char file[1024];
    _get_file(file, mission_pack);

    char id[100];
    _get_id(id, best_times);

    char record[1024];
    strcpy(record, id);
    for (int i = 0; i < NUM_BEST_TIMES; i++)
    {
        char buf[100];
        sprintf(buf, " %f", best_times->times[i]);
        strcat(record, buf);
    }
    strcat(record, " end");

    return record_file_set_record(file, id, record);
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
        return 0;

    for (int i = NUM_BEST_TIMES - 1; i > idx_beaten; i--)
    {
        best_times->times[i] = best_times->times[i - 1];
    }

    best_times->times[idx_beaten] = time;

    return 1;
}