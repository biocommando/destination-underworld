#pragma once

typedef struct
{
    int count;
    char flags[100];
} command_file_RequiredFlags;

#define SET_command_file_RequiredFlags_FLAG(idx, flags) \
    flags[(idx) / 8] |= (1 << ((idx) % 8))

#define GET_command_file_RequiredFlags_FLAG(idx, flags) \
    (flags[(idx) / 8] & (1 << ((idx) % 8))) != 0

#define IF_command_file_RequiredFlags_NOT_SET(ptr, expression)             \
    do                                                                     \
    {                                                                      \
        int count = 0;                                                     \
        for (int i = 0; i < 100 * 8 && i < (ptr)->count; i++)              \
        {                                                                  \
            count += GET_command_file_RequiredFlags_FLAG(i, (ptr)->flags); \
        }                                                                  \
        if (count != (ptr)->count)                                         \
        {                                                                  \
            expression;                                                    \
        }                                                                  \
    } while (0)
