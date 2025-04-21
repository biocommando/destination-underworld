#include "midi_playback.h"
#include "allegro5/allegro.h"
#include "duConstants.h"
#include <stdio.h>
#include <string.h>

#define MAX_PLAYLIST_LEN 128

static MidiPlayer midi_player;
static MidiFile midi_file;
static int midi_file_ready = 0;

struct playlist_entry
{
    char *path;
    char *meta_f;
};

static struct playlist_entry playlist[MAX_PLAYLIST_LEN];
static int playlist_entries = 0;
static int current_playlist_entry = -1;

static int check_file_type(const char *fn, const char *type)
{
    size_t type_len = strlen(type);
    size_t fn_len = strlen(fn);
    if (fn_len < type_len)
        return -1;
    for (size_t i = 0; i < type_len; i++)
    {
        if (fn[fn_len - type_len + i] != type[i])
            return -2;
    }
    return 0;
}

static void load_playlist()
{
    playlist_entries = 0;
    current_playlist_entry = -1;
    ALLEGRO_FS_ENTRY *e = al_create_fs_entry(DATADIR "midi-music");
    al_open_directory(e);
    ALLEGRO_FS_ENTRY *next;

    while (1)
    {
        next = al_read_directory(e);
        if (!next)
            break;
        const char *fname = al_get_fs_entry_name(next);
        if (check_file_type(fname, ".mid") == 0)
        {
            char *meta_f_name = (char *)malloc(strlen(fname) + 9 + 1);
            sprintf(meta_f_name, "%s_meta.ini", fname);
            FILE *meta_f = fopen(meta_f_name, "r");
            if (meta_f)
            {
                fclose(meta_f);
                struct playlist_entry *pe = &playlist[playlist_entries];
                pe->path = (char *)malloc(strlen(fname) + 1);
                strcpy(pe->path, fname);
                pe->meta_f = meta_f_name;
                playlist_entries++;
                if (playlist_entries == MAX_PLAYLIST_LEN)
                    break;
            }
            else
            {
                free(meta_f_name);
            }
        }
    }
    al_close_directory(e);
    al_destroy_fs_entry(e);
}

void init_midi_playback(float sample_rate)
{
    memset(playlist, 0, sizeof(playlist));
    init_midi_player(&midi_player, sample_rate);
    load_playlist();
}

void free_midi_playback()
{
    for (int i = 0; i < MAX_PLAYLIST_LEN; i++)
    {
        struct playlist_entry *pe = &playlist[i];
        free(pe->path);
        free(pe->meta_f);
    }
    if (midi_file_ready)
        free_midi_file(&midi_file);
    free_midi_player(&midi_player);
}

MidiPlayer *get_midi_player()
{
    return &midi_player;
}

void next_midi_track(int index)
{
    if (playlist_entries == 0)
        return;
    if (midi_file_ready)
        free_midi_file(&midi_file);
    if (index >= 0)
        current_playlist_entry = index;
    else
        current_playlist_entry++;
    if (current_playlist_entry >= playlist_entries)
        current_playlist_entry = 0;
    Synth_read_instruments(&midi_player.synth, playlist[current_playlist_entry].meta_f);
    init_midi_file(&midi_file);
    midi_file.sample_rate = midi_player.sample_rate;
    read_midi_file(playlist[current_playlist_entry].path, &midi_file);
    midi_file_ready = 1;
    midi_player_set_midi_file(&midi_player, &midi_file);
}

static int randomize_midi_playlist_sorter([[maybe_unused]] const void *a, [[maybe_unused]] const void *b)
{
    return rand() & 1 ? 1 : -1;
}

void randomize_midi_playlist()
{
    qsort(playlist, playlist_entries, sizeof(struct playlist_entry), randomize_midi_playlist_sorter);
}

const char *get_midi_playlist_entry_file_name(int index)
{
    if (index < 0)
    {
        if (current_playlist_entry < 0)
            return NULL;
        index = current_playlist_entry;
    }
    if (index >= playlist_entries)
        return NULL;

    char *ret = playlist[index].path;
    for (char *c = ret; *c; c++)
    {
        if (*c == '\\' || *c == '/')
            ret = c + 1;
    }
    return ret;
}