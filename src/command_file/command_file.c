#include "command_file.h"
#include "strescape.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// LINE SYNTAX:
// # comment
// :label
// command:"arg" "arg" ...

static void parse_parameters(char *params_start, command_file_DispatchDto *dto)
{
    if (!params_start)
        return;
    int num_args = 0;
    char *p = params_start - 1;
    while (p && num_args < command_file_PARAMS_MAX)
    {
        p = strstr(p + 1, "\"");
        if (!p)
            break;
        p++;
        char *pp = p;
        dto->parameters[num_args++] = p;
        p = strstr(p, "\"");
        if (p)
            *p = 0;
        strescape_inplace(pp, '\\', "\\\\'\"n\n", 6);
    }
}

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
        int line_len = strlen(line);
        for (int i = line_len - 1; line[i] == '\n' || line[i] == '\r'; i--)
        {
            line[i] = 0;
            line_len--;
        }
        if (line_len == command_file_LINE_MAX - 1) // Too long line
        {
            // Read rest of the line
            char c;
            while (fread(&c, 1, 1, f) == 1 && c != '\n' && c != '\r');
            // Go to the start of the new line sequence, let the normal
            // process handle the "ghost" empty line
            fseek(f, -1, SEEK_CUR);
            continue;
        }
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
        char *p;
        if ((p = strstr(line, ":")))
        {
            *p = 0;
            parse_parameters(p + 1, &dto);
        }
        else if ((p = strstr(line, "\"")))
        {
            parse_parameters(p, &dto);
            *line = 0;
        }
        strescape_inplace(line, '\\', "\\\\.:'\"", 6);
        dispatch(&dto);
    }
    fclose(f);
    return 0;
}