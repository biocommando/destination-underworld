#pragma once

#define command_file_LINE_MAX 256
#define command_file_PARAMS_MAX 32

typedef struct
{
    const char *command;
    const char *parameters[command_file_PARAMS_MAX];
    char skip_label[command_file_LINE_MAX];
    void *state;
} command_file_DispatchDto;

int read_command_file(const char *filename, void (*dispatch)(command_file_DispatchDto *), void *state);