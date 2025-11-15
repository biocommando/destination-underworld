#pragma once

// Internal representation of a variable
typedef struct
{
    char name[256];
    char value[256];
    int read_only;
} Variable;

// Collection of variables
typedef struct
{
    Variable *vars;
    unsigned num_vars;
} VarState;

// Create empty variables
VarState create_VarState();

// Get value of a variable or NULL if variable with that name doesn't exist
const char *get_var(const char *name, const VarState *state);

// Set value of a variable.
// Returns 0 if successful, -1 otherwise (name or value too long or variable is readonly).
int set_var(const char *name, const char *value, VarState *state);

// Set variable as readonly.
int set_var_readonly(const char *name, VarState *state);

// Free all variables and reset to initial state.
void free_VarState(VarState *state);
