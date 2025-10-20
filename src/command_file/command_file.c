#include "command_file.h"
#include "strescape.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LINE SYNTAX:
// # comment
// :label
// command:"arg" "arg" ...

int read_command_file(const char *filename, void (*dispatch)(command_file_DispatchDto *), void *state)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return 1;
    char line[command_file_LINE_MAX];
    command_file_DispatchDto dto;
    dto.state = state;
    *dto.skip_label = 0;
    dto.command = line;
    while (!feof(f))
    {
        if (!fgets(line, command_file_LINE_MAX, f))
            break;
        for (int i = strlen(line) - 1; line[i] == '\n' || line[i] == '\r'; i--)
            line[i] = 0;
        if (!*line || *line == '#')
            continue;
        if (*line == ':')
        {
            if (!strcmp(dto.skip_label, line + 1))
                *dto.skip_label = 0;
            continue;
        }
        if (*dto.skip_label)
            continue;
        memset(dto.parameters, 0, sizeof(dto.parameters));
        char *p = strstr(line, ":");
        if (p)
        {
            *p = 0;
            int num_args = 0;
            while (p && num_args < 32)
            {
                p = strstr(p + 1, "\"");
                if (!p)
                    break;
                p++;
                char *pp = p;
                dto.parameters[num_args++] = p;
                p = strstr(p, "\"");
                if (p)
                    *p = 0;
                strescape_inplace(pp, '\\', "'\"n\n", 4);
            }
        }
        dispatch(&dto);
    }
    fclose(f);
    return 0;
}