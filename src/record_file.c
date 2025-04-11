#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "record_file.h"

#define REC_MAX_LENGTH 1024

struct mem_record
{
    char value[REC_MAX_LENGTH];
};

struct mem_record_db
{
    int sz;
    struct mem_record *recs;

    int dirty;

    char current_file[1024];
};

static struct mem_record_db state;
static int init_done = 0;

static inline void init()
{
    if (init_done)
    {
        return;
    }
    init_done = 1;

    memset(&state, 0, sizeof(state));
}

void record_file_flush()
{
    init();
    if (state.dirty)
    {
        FILE *f = fopen(state.current_file, "w");
        // Optional meta information in the beginning of the file
        fprintf(f, "_record_count %d\n", state.sz);
        for (int i = 0; i < state.sz; i++)
        {
            const struct mem_record *rec = &state.recs[i];
            fprintf(f, "%s\n", rec->value);
        }
        fclose(f);
    }

    free(state.recs);
    memset(&state, 0, sizeof(state));
}

void record_file_read(const char *file)
{
    record_file_flush();
    strncpy(state.current_file, file, 1023);

    char line[REC_MAX_LENGTH];
    FILE *f = fopen(file, "r");
    if (f == NULL)
    {
        return;
    }
    int line_idx = 0;
    while (fgets(line, sizeof(line), f))
    {
        if (line[0] == '#')
        {
            continue;
        }
        size_t line_len = strlen(line);
        if (line[line_len - 1] == '\n')
        {
            line_len--;
            line[line_len] = '\0';
        }
        if (line_len == 0)
            continue;
        if (state.recs == NULL && sscanf(line, "_record_count %d", &state.sz) == 1)
        {
            state.recs = (struct mem_record *)calloc(state.sz, sizeof(struct mem_record));
            continue;
        }
        if (line_idx + 1 > state.sz)
        {
            state.sz = line_idx + 1;
            size_t sz = sizeof(struct mem_record) * state.sz;
            state.recs = (struct mem_record *)realloc(state.recs, sz);
        }
        memcpy(&state.recs[line_idx], line, REC_MAX_LENGTH);
        line_idx++;
    }
    fclose(f);
}

// Returns the index of the record if found, -1 otherwise
static int get_record_index(const char *file, const char *id)
{
    if (strcmp(file, state.current_file) != 0)
    {
        record_file_read(file);
    }

    char key[REC_MAX_LENGTH];
    for (int i = 0; i < state.sz; i++)
    {
        key[0] = 0;
        sscanf(state.recs[i].value, "%s", key);
        if (!strcmp(id, key))
        {
            return i;
        }
    }

    return -1;
}

int record_file_get_record(const char *file, const char *id, char *record, size_t sz)
{
    int rec_idx = get_record_index(file, id);
    if (rec_idx >= 0)
    {
        memcpy(record, state.recs[rec_idx].value, sz > REC_MAX_LENGTH ? REC_MAX_LENGTH : sz);
        return 0;
    }
    return 1;
}

int record_file_set_record(const char *file, const char *id, const char *record)
{
    if (strlen(record) >= REC_MAX_LENGTH)
    {
        return 1;
    }
    int rec_idx = get_record_index(file, id);
    if (rec_idx == -1)
    {
        rec_idx = state.sz;
        state.sz++;
        size_t sz = sizeof(struct mem_record) * state.sz;
        state.recs = (struct mem_record *)realloc(state.recs, sz);
    }
    strcpy(state.recs[rec_idx].value, record);

    state.dirty = 1;
    return 0;
}

int record_file_set_record_f(const char *file, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char record[REC_MAX_LENGTH];
    vsnprintf(record, REC_MAX_LENGTH, format, args);
    char id[REC_MAX_LENGTH];
    sscanf(record, "%s", id);
    return record_file_set_record(file, id, record);
}

int record_file_scanf(const char *file, const char *id, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char record[REC_MAX_LENGTH];
    int err = record_file_get_record(file, id, record, REC_MAX_LENGTH);
    if (err)
        return 0;
    return vsscanf(record, format, args);
}