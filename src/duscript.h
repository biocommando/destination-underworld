#pragma once

#define DU_SCRIPT_NUM_VARS 100
#define DU_SCRIPT_MAX_STR_LEN 128

typedef struct
{
    char name[DU_SCRIPT_MAX_STR_LEN];
    char value[DU_SCRIPT_MAX_STR_LEN];
    int read_only;
} DuScriptVariable;

typedef struct
{
    int count;
    DuScriptVariable variables[DU_SCRIPT_NUM_VARS];
    char goto_label[DU_SCRIPT_MAX_STR_LEN];
} DuScriptState;

DuScriptState du_script_init();
// Returns:
// - 0 = expect next line
// - 1 = expect start of file
// - -1 = command not parsed
int du_script_execute_line(DuScriptState *state, const char *line);

DuScriptVariable *du_script_variable(DuScriptState *state, const char *name);