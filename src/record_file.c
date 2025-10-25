#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "command_file/generated/dispatch_record_file.h"
#include "record_file.h"
#include "linked_list.h"

#define REC_MAX_LENGTH command_file_LINE_MAX

struct new_mem_record_param
{
    char value[REC_MAX_LENGTH];
};

struct new_mem_record
{
    char key[REC_MAX_LENGTH];
    LinkedList params;
};

struct new_mem_record_db
{
    LinkedList recs;

    int dirty;

    char current_file[1024];

    struct new_mem_record *current;
    LinkedList_it_state read_iter_state;
    char access_mode;
};

static struct new_mem_record_db state;
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
    struct new_mem_record *rec;
    LINKED_LIST_FOR_EACH(&state.recs, struct new_mem_record, rec, 0)
    {
        fprintf(f, "%s:", rec->key);
        struct new_mem_record_param *param;
        LINKED_LIST_FOR_EACH(&rec->params, struct new_mem_record_param, param, 0)
        {
            fprintf(f, "\"%s\"", param->value);
        }
        fprintf(f, "\n");
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

    struct new_mem_record *rec;
    LINKED_LIST_FOR_EACH(&state.recs, struct new_mem_record, rec, 0)
    {
        linked_list_clear(&rec->params);
    }

    linked_list_clear(&state.recs);
    memset(&state, 0, sizeof(state));
}

static void add_parameter(LinkedList *lst, const char *value)
{
    struct new_mem_record_param *param = LINKED_LIST_ADD(lst, struct new_mem_record_param);
    strcpy(param->value, value);
}

void dispatch__handle_record_file_default(struct record_file_default_DispatchDto *dto)
{
    struct new_mem_record *rec = LINKED_LIST_ADD(&state.recs, struct new_mem_record);
    strcpy(rec->key, dto->command);
    for (int i = 0; i < command_file_PARAMS_MAX && dto->parent->parameters[i]; i++)
    {
        add_parameter(&rec->params, dto->parent->parameters[i]);
    }
}

static void record_file_read(const char *file)
{
    record_file_flush();
    strncpy(state.current_file, file, 1023);
    RecordFileState rec_state;
    rec_state.line_idx = 0;
    read_command_file(file, dispatch__record_file, &rec_state);
}

// Returns the index of the record if found, -1 otherwise
static struct new_mem_record *get_record_index(const char *file, const char *id)
{
    if (strcmp(file, state.current_file) != 0)
    {
        record_file_read(file);
    }

    struct new_mem_record *rec;
    LINKED_LIST_FOR_EACH(&state.recs, struct new_mem_record, rec, 0)
    {
        if (!strcmp(id, rec->key))
        {
            return rec;
        }
    }

    return NULL;
}

static int record_file_find(const char *file, const char *id, int create)
{
    struct new_mem_record *rec = get_record_index(file, id);
    if (!rec && create && strlen(id) < REC_MAX_LENGTH)
    {
        rec = LINKED_LIST_ADD(&state.recs, struct new_mem_record);
        strcpy(rec->key, id);
    }

    if (rec)
    {
        state.current = rec;
        if (create)
        {
            linked_list_clear(&rec->params);
            state.dirty = 1;
        }
        else
            state.read_iter_state = linked_list_iteration_start(&rec->params);
        state.access_mode = create ? 'w' : 'r';
    }
    return rec ? 0 : -1;
}

int record_file_find_and_modify(const char *file, const char *id)
{
    return record_file_find(file, id, 1);
}

int record_file_find_and_read(const char *file, const char *id)
{
    return record_file_find(file, id, 0);
}

int record_file_add_param(const char *param)
{
    if (!state.current || state.access_mode != 'w' || state.current->params.count >= command_file_PARAMS_MAX || strlen(param) >= REC_MAX_LENGTH)
        return -1;

    add_parameter(&state.current->params, param);
    return 0;
}

int record_file_add_int_param(int param)
{
    char sval[20];
    sprintf(sval, "%d", param);
    return record_file_add_param(sval);
}

int record_file_add_float_param(float param)
{
    char sval[20];
    sprintf(sval, "%f", param);
    return record_file_add_param(sval);
}

const char *record_file_next_param()
{
    if (state.access_mode != 'r')
        return NULL;
    const struct new_mem_record_param *param = (const struct new_mem_record_param *)linked_list_iterate(&state.read_iter_state);
    if (!param)
        return NULL;
    return param->value;
}

int record_file_next_param_as_int(int err)
{
    const char *value = record_file_next_param();
    int ret = err;
    if (value)
        sscanf(value, "%d", &ret);
    return ret;
}

float record_file_next_param_as_float(float err)
{
    const char *value = record_file_next_param();
    float ret = err;
    if (value)
        sscanf(value, "%f", &ret);
    return ret;
}