#include "variables.h"

#include <stdlib.h>
#include <string.h>


VarState create_VarState()
{
    VarState s;
    memset(&s, 0, sizeof(s));
    return s;
}

static long get_var_idx(const char *name, const VarState *state)
{
    for (unsigned i = 0; i < state->num_vars; i++)
    {
        if (!strcmp(name, state->vars[i].name))
        {
            return i;
        }
    }
    return -1;
}


const char *get_var(const char *name, const VarState *state)
{
    long idx = get_var_idx(name, state);
    if (idx < 0)
        return NULL;
    return state->vars[idx].value;
}

int set_var(const char *name, const char *value, VarState *state)
{
    long idx = get_var_idx(name, state);
    if (idx < 0)
    {
        if (strlen(name) >= 256)
            return -1;
        unsigned new_idx = state->num_vars;
        state->num_vars++;
        state->vars = realloc(state->vars, sizeof(Variable) * state->num_vars);
        memset(&state->vars[new_idx], 0, sizeof(Variable));
        strcpy(state->vars[new_idx].name, name);
        idx = (long)new_idx;
    }

    if (state->vars[idx].read_only || strlen(value) >= 256)
        return -1;

    strcpy(state->vars[idx].value, value);
    return 0;
}

int set_var_readonly(const char *name, VarState *state)
{
    long idx = get_var_idx(name, state);
    if (idx < 0)
        return -1;
    state->vars[idx].read_only = 1;
    return 0;
}

void free_VarState(VarState *state)
{
    if (state->vars)
    {
        free(state->vars);
        state->vars = NULL;
        state->num_vars = 0;
    }
}