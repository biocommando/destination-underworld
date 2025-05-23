#include "duscript.h"

#include "logging.h"

#include <stdio.h>
#include <string.h>

DuScriptState du_script_init()
{
    DuScriptState s;
    s.count = 0;
    *s.goto_label = 0;
    return s;
}

int du_script_execute_line(DuScriptState *state, const char *line)
{
    DuScriptVariable *var = NULL;
    if (*line != '*')
        return *state->goto_label ? 0 : -1;
    if (line[1] == '@' && *state->goto_label)
    {
        char lbl[DU_SCRIPT_MAX_STR_LEN] = "";
        sscanf(line + 2, "%s", lbl);
        LOG_TRACE("matching label <%s> == <%s>\n", lbl, state->goto_label);
        if (!strcmp(state->goto_label, lbl))
        {
            *state->goto_label = 0;
        }
        return 0;
    }
    if (*state->goto_label || line[1] == '@')
    {
        return 0;
    }
    if (line[1] == '=' && strlen(line) < DU_SCRIPT_MAX_STR_LEN)
    {
        const char *var_start = line + 2;
        const char *var_end = strstr(var_start, "\"");
        if (!var_end)
            return 0;
        const char *val_end = strstr(var_end + 1, "\"");
        if (!val_end)
            return 0;
        char name[DU_SCRIPT_MAX_STR_LEN];
        int len = var_end - var_start;
        memcpy(name, var_start, len);
        name[len] = 0;
        var = du_script_variable(state, name);
        if (!var || var->read_only)
            return 0;
        len = val_end - (var_end + 1);
        memcpy(var->value, var_end + 1, len);
        var->value[len] = 0;
        return 0;
    }
    if (line[1] == '?')
    {
        const char *p = line + 2;
        while (p)
        {
            char goto_label[DU_SCRIPT_MAX_STR_LEN] = "";
            char name[DU_SCRIPT_MAX_STR_LEN] = "";
            char val[DU_SCRIPT_MAX_STR_LEN] = "";
            char op = 0;
            if (sscanf(p, "%s %c %s %s", name, &op, val, goto_label) != 4)
                return 0;
            LOG_TRACE("Compare %s %c %s : %s\n", name, op, val, goto_label);
            var = du_script_variable(state, name);
            int same = var && !strcmp(var->value, val);
            if ((op == '=' && same) || (op == '!' && !same))
            {
                LOG_TRACE("Compare result true\n");
                if (!strcmp("and", goto_label))
                {
                    p = strstr(p, " and ");
                    if (p)
                    {
                        p += 5;
                    }
                }
                else
                {
                    LOG_TRACE("condition ok -> jump to <%s>\n", goto_label);
                    strcpy(state->goto_label, goto_label);
                    return *goto_label == '+' ? 0 : 1;
                }
            }
            else
            {
                LOG_TRACE("Compare result false\n");
                break;
            }
        }
        return 0;
    }
    if (line[1] == '>')
    {
        sscanf(line + 2, "%s", state->goto_label);
        LOG_TRACE("Goto <%s>\n", state->goto_label);
        return *state->goto_label == '+' ? 0 : 1;
    }

    return -1;
}

DuScriptVariable *du_script_variable(DuScriptState *state, const char *name)
{
    for (int i = 0; i < state->count; i++)
    {
        DuScriptVariable *var = &state->variables[i];
        if (!strcmp(name, var->name))
        {
            return var;
        }
    }
    if (state->count < DU_SCRIPT_NUM_VARS)
    {
        DuScriptVariable *var = &state->variables[state->count++];
        strcpy(var->name, name);
        *var->value = 0;
        var->read_only = 0;
        return var;
    }
    return NULL;
}