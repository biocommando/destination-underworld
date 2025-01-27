#pragma once

#define DU_SCRIPT_NUM_VARS 100
#define DU_SCRIPT_MAX_STR_LEN 256

// Scripting engine variable.
typedef struct
{
    // Variable name
    char name[DU_SCRIPT_MAX_STR_LEN];
    // Variable value (always string value, other types must be parsed)
    char value[DU_SCRIPT_MAX_STR_LEN];
    // Set to 1 to prevent script file from overriding the default value
    int read_only;
} DuScriptVariable;

// Scripting engine state. The fields are supposed to be for internal logic only.
// Accessing the state is done using the functions in this file.
typedef struct
{
    // Variable count
    int count;
    DuScriptVariable variables[DU_SCRIPT_NUM_VARS];
    // Current goto label (= all lines will be skipped before finding the label)
    char goto_label[DU_SCRIPT_MAX_STR_LEN];
} DuScriptState;

/*
 * Creates a new state for script execution.
 */
DuScriptState du_script_init();

/*
 Execute a single line in the script.
 The lines that the script engine can parse start with an asterisk.
 Other lines are expected to be data lines. So basically the script only
 determines what data is read (due to conditional branching), and pre-defined
 variables can be used for setting game properties.

 Syntax:
```
*@[label]                                   goto label
*=[variable]"[value]"                       set variable to value
*?[variable] [operator] [value] [label]     if variable is equal / not equal to value go to label
                                            operators:
                                                =       equals
                                                !       not equal
*>[label]                                   go to label. If label starts with +, only search forward
```
 Returns:
 - 0 = expect next line
 - 1 = expect start of file
 - -1 = command not parsed
*/
int du_script_execute_line(DuScriptState *state, const char *line);

/*
 * Gets a variable from the state with the provided name. If the variable didn't exist already,
 * creates a new variable.
 *
 * Returns NULL if there are too many variables in use.
 */
DuScriptVariable *du_script_variable(DuScriptState *state, const char *name);