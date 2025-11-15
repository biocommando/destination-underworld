#pragma once

/* This module provides a simple line-oriented command file parser. Each line
 * of the input file can represent:
 *
 *   • A comment line starting with '#'
 *   • A label definition beginning with ':labelName'
 *   • A command line of the form:
 *         command:"arg1" "arg2" ...
 *
 * The parser invokes a user-supplied callback for each parsed command and
 * extracts parameters enclosed in double quotes. Escape sequences inside
 * arguments are processed using strescape_inplace(). Following escape sequences
 * are supported:
 * \n = new line
 * \' = "
 * \\ = backslash
 * For command name only:
 * \. = :
 */

// Maximum length of a single input line (including null terminator).
#define command_file_LINE_MAX 256

//  Maximum number of parsed parameters allowed per command line.
#define command_file_PARAMS_MAX 32

// A data transfer object passed to the dispatch callback when a command line is parsed.
typedef struct
{
    /* Pointer to the command name extracted from the line. This is
       the portion before any ':'. */
    const char *command;
    /* Array of pointers to each parsed parameter string. All strings
       are null-terminated and have escape sequences processed.
       Unused entries are NULL. */
    const char *parameters[command_file_PARAMS_MAX];
    /* A buffer used internally for controlling label-based skipping.
       If non-empty, parsing will skip commands until a matching
       ':label' line is encountered. */
    char skip_label[command_file_LINE_MAX];
    /* User-provided pointer passed directly through to the dispatch
       callback, allowing the caller to maintain context or state. */
    void *state;
} command_file_DispatchDto;

/*    Parses a command file line-by-line and invokes a callback for each
 *    command encountered.
 *
 *    Parameters:
 *      filename
 *          Path to the command file to be read.
 *
 *      dispatch(dto)
 *          User-supplied function called for each parsed command. The dto
 *          argument contains the parsed command name, parameters, and user
 *          state.
 *
 *      state
 *          Arbitrary pointer forwarded to dto->state for use in the dispatch
 *          function.
 *
 *    Returns:
 *      0 on success.
 *      Non-zero if file opening fails. */
int read_command_file(const char *filename, void (*dispatch)(command_file_DispatchDto *), void *state);
