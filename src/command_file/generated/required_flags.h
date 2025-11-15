#pragma once

// Maximum number of flags. Should be divisible by 8.
#define command_file_RequiredFlags_MAX_COUNT 800

// Flags for verifying that all required fields have been set
typedef struct
{
    // Number of flags to set
    int count;
    // Flags (should be initialized to zero, 1 = flag set)
    char flags[command_file_RequiredFlags_MAX_COUNT / 8];
} command_file_RequiredFlags;

// Set a flag to 1 at selected index
#define SET_command_file_RequiredFlags_FLAG(idx, flags) \
    flags[(idx) / 8] |= (1 << ((idx) % 8))

// Returns the flag value at selected index
#define GET_command_file_RequiredFlags_FLAG(idx, flags) \
    (flags[(idx) / 8] & (1 << ((idx) % 8))) != 0

// Checks if all the flags (up to command_file_RequiredFlags.count) have been set.
// If not, execute the expression.
#define IF_command_file_RequiredFlags_NOT_SET(ptr, expression)                             \
    do                                                                                     \
    {                                                                                      \
        int count = 0;                                                                     \
        for (int i = 0; i < command_file_RequiredFlags_MAX_COUNT && i < (ptr)->count; i++) \
        {                                                                                  \
            count += GET_command_file_RequiredFlags_FLAG(i, (ptr)->flags);                 \
        }                                                                                  \
        if (count != (ptr)->count)                                                         \
        {                                                                                  \
            expression;                                                                    \
        }                                                                                  \
    } while (0)
