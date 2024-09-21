#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

struct mem_record_db rec_db_state;
int rec_db_state_init_done = 0;

void record_file_init()
{
    if (rec_db_state_init_done)
    {
        return;
    }
    rec_db_state_init_done = 1;

    memset(&rec_db_state, 0, sizeof(rec_db_state));
}

void record_file_flush()
{
    record_file_init();
    if (rec_db_state.dirty)
    {
        FILE *f = fopen(rec_db_state.current_file, "w");
        // Optional meta information in the beginning of the file
        fprintf(f, "_record_count %d\n", rec_db_state.sz);
        for (int i = 0; i < rec_db_state.sz; i++)
        {
            const struct mem_record *rec = &rec_db_state.recs[i];
            fprintf(f, "%s\n", rec->value);
        }
        fclose(f);
    }

    free(rec_db_state.recs);
    memset(&rec_db_state, 0, sizeof(rec_db_state));
}

int record_file_read(const char *file)
{
    record_file_flush();
    strncpy(rec_db_state.current_file, file, 1024);

    char line[REC_MAX_LENGTH];
    FILE *f = fopen(file, "r");
    int line_idx = 0;
    while (fgets(line, sizeof(line), f))
    {
        if (line[strlen(line) - 1] == '\n')
        {
            line[strlen(line) - 1] = '\0';
        }
        if (rec_db_state.recs == NULL && sscanf(line, "_record_count %d", &rec_db_state.sz) == 1)
        {
            rec_db_state.recs = (struct mem_record*) calloc(rec_db_state.sz, sizeof(struct mem_record));
            continue;
        }
        if (line_idx + 1 > rec_db_state.sz)
        {
            rec_db_state.sz = line_idx + 1;
            size_t sz = sizeof(struct mem_record) * rec_db_state.sz;
            rec_db_state.recs = (struct mem_record*) realloc(rec_db_state.recs, sz);
        }
        memcpy(&rec_db_state.recs[line_idx], line, REC_MAX_LENGTH);
        line_idx++;
    }
    fclose(f);
}

// Returns the index of the record if found, -1 otherwise
int record_file_db_get_record(const char *file, const char *id)
{
    if (strcmp(file, rec_db_state.current_file) != 0)
    {
        record_file_read(file);
    }

    char key[REC_MAX_LENGTH];
    for (int i = 0; i < rec_db_state.sz; i++)
    {
        key[0] = 0;
        sscanf(rec_db_state.recs[i].value, "%s", key);
        if (!strcmp(id, key))
        {
            return i;
        }
    }

    return -1;
}

int record_file_get_record(const char *file, const char *id, char *record, size_t sz)
{
    int rec_idx = record_file_db_get_record(file, id);
    if (rec_idx >= 0)
    {
        memcpy(record, rec_db_state.recs[rec_idx].value, sz > REC_MAX_LENGTH ? REC_MAX_LENGTH : sz);
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
    int rec_idx = record_file_db_get_record(file, id);
    if (rec_idx == -1)
    {
        rec_idx = rec_db_state.sz;
        rec_db_state.sz++;
        size_t sz = sizeof(struct mem_record) * rec_db_state.sz;
        rec_db_state.recs = (struct mem_record*) realloc(rec_db_state.recs, sz);
    }
    strcpy(rec_db_state.recs[rec_idx].value, record);

    rec_db_state.dirty = 1;
    return 0;
}