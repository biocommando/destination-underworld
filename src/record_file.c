#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "command_file/generated/dispatch_record_file.h"
#include "record_file.h"
#include "linked_list.h"

#define REC_MAX_LENGTH command_file_LINE_MAX

struct mem_record
{
    char value[REC_MAX_LENGTH];
    size_t sz;
};

struct mem_record_db
{
    LinkedList recs;

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

static inline void write_current_db_to_file()
{
    FILE *f = fopen(state.current_file, "w");
    if (!f)
        return;
    struct mem_record *rec;
    LINKED_LIST_FOR_EACH(&state.recs, struct mem_record, rec, 0)
    {
        fprintf(f, "%s\n", rec->value);
    }
    fclose(f);
}

void record_file_flush()
{
    init();
    if (state.dirty)
    {
        write_current_db_to_file();
    }

    linked_list_clear(&state.recs);
    memset(&state, 0, sizeof(state));
}

void dispatch__handle_record_file_default(struct record_file_default_DispatchDto *dto)
{
    struct mem_record *rec = LINKED_LIST_ADD(&state.recs, struct mem_record);
    int len = strlen(dto->command) + 1;
    memcpy(&rec->value, dto->command, len);
    rec->sz = len;
}

void record_file_read(const char *file)
{
    record_file_flush();
    strncpy(state.current_file, file, 1023);
    RecordFileState rec_state;
    rec_state.line_idx = 0;
    read_command_file(file, dispatch__record_file, &rec_state);
}

// Returns the index of the record if found, -1 otherwise
static struct mem_record *get_record_index(const char *file, const char *id)
{
    if (strcmp(file, state.current_file) != 0)
    {
        record_file_read(file);
    }

    char key[REC_MAX_LENGTH];
    struct mem_record *rec;
    LINKED_LIST_FOR_EACH(&state.recs, struct mem_record, rec, 0)
    {
        key[0] = 0;
        sscanf(rec->value, "%s", key);
        if (!strcmp(id, key))
        {
            return rec;
        }
    }

    return NULL;
}

int record_file_get_record(const char *file, const char *id, char *record, size_t sz)
{
    struct mem_record *rec = get_record_index(file, id);
    if (rec && rec->sz <= sz)
    {
        memcpy(record, rec->value, sz > REC_MAX_LENGTH ? REC_MAX_LENGTH : sz);
        return 0;
    }
    return 1;
}

int record_file_set_record(const char *file, const char *id, const char *record)
{
    size_t length = strlen(record);
    if (length >= REC_MAX_LENGTH)
    {
        return 1;
    }

    size_t id_length = strlen(id);
    if (id_length > length || memcmp(id, record, id_length))
    {
        return 1;
    }

    struct mem_record *rec = get_record_index(file, id);
    if (rec == NULL)
    {
        rec = LINKED_LIST_ADD(&state.recs, struct mem_record);
    }
    strcpy(rec->value, record);
    rec->sz = length + 1;

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