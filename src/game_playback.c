#include "game_playback.h"
#include "record_file.h"
#include "duConstants.h"

#include <string.h>

static int record_mode = RECORD_MODE_NONE;
struct game_playback_event
{
    long time_stamp;
    long key_mask;
    int end;
};

static char filename[256] = "";
static int event_index = 0;

void game_playback_init()
{
    event_index = 0;
}

static inline void format_id(char *id)
{
    sprintf(id, "event_%d", event_index);
}

static void get_event(struct game_playback_event *evt)
{
    char id[20], value[100];
    format_id(id);
    if (record_file_get_record(filename, id, value, sizeof(value)) == 0)
    {
        sscanf(value, "%*s end=%d time=%ld keys=%ld",
               &evt->end, &evt->time_stamp, &evt->key_mask);
    }
}

void game_playback_set_event(const struct game_playback_event *evt)
{
    char id[20], value[100];
    format_id(id);
    record_file_set_record_f(filename, "%s end=%d time=%ld keys=%ld",
                             id, evt->end, evt->time_stamp, evt->key_mask);
}

long game_playback_get_time_stamp()
{
    struct game_playback_event tmp;
    tmp.end = 0;
    tmp.time_stamp = 0;
    get_event(&tmp);
    return tmp.end ? -1 : tmp.time_stamp;
}

long game_playback_get_key_mask()
{
    struct game_playback_event tmp;
    get_event(&tmp);
    return tmp.key_mask;
}

void game_playback_add_key_event(long time_stamp, long key_mask)
{
    struct game_playback_event tmp;
    tmp.end = 0;
    tmp.time_stamp = time_stamp;
    tmp.key_mask = key_mask;
    game_playback_set_event(&tmp);
}

void game_playback_add_end_event()
{
    if (event_index == 0)
    {
        // Logic is easier to write if we can assume that there's at least one event
        // in the file
        game_playback_add_key_event(0, 0);
        game_playback_next();
    }
    struct game_playback_event tmp = {0, 0, 0};
    tmp.end = 1;
    game_playback_set_event(&tmp);
}

void game_playback_next()
{
    event_index++;
}

inline int *get_playback_mode()
{
    return &record_mode;
}

inline void game_playback_set_filename(const char *fn)
{
    if (strlen(fn) >= sizeof(filename))
        return;
    strcpy(filename, fn);
}

inline const char *game_playback_get_filename()
{
    return filename;
}