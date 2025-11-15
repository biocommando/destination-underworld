# Command File Dispatch Generator

This tool generates C source code for handling **command_file**–compatible command scripts.
Given a JSON definition, it produces:

* `dispatch_<CommandSet>.h`
* `dispatch_<CommandSet>.c`

Each generated dispatcher reads parsed lines from `command_file_DispatchDto` (produced by `read_command_file()`), validates parameters, builds typed DTOs, and invokes user-defined handlers.

---

## Overview

The generator transforms a JSON description of command formats into C code that:

✔ Validates command parameters
✔ Converts parameters into C types (`int`, `double`, `const char *`)
✔ Generates per-command DTO structs
✔ Generates dispatch logic for each command
✔ Generates optional:

* Default handlers
* Required field tracking
* Parameter validators
* Debug print helpers
  ✔ Produces field-setter helpers if enabled

The result is a type-safe, validated interface for reading commands from text files.

---

## Usage

```
node generate.js <definition.json>
```

Output files:

```
dispatch_<CommandSet>.h
dispatch_<CommandSet>.c
```

Each command set in the JSON generates its own pair of `.c/.h` files.

---

## JSON File Structure

A JSON file contains one or more **command sets**, each producing its own dispatcher.

```json
{
    "MyCommands": {
        ":config": {
            "state": "MyState",
            "prefix": "my_",
            "debug": true,
            "default": true,
            "defaultrequired": false,
            "list": false,
            "fieldsetter": false
        },

        "Move": {
            "x": "int",
            "y": "int",
            "validate:x": { "type": "range", "min": 0, "max": 100 },
            ":required": true
        },

        "Say": {
            "message": "str",
            "validate:message": { "type": "length", "min": 1, "max": 64 }
        }
    }
}
```

---

## Config Options

### `state` (required)

Name of the state struct used by command handlers.
The generator includes:

```
#include "state_<state>.h"
```

You must write this by hand. The file must declare a type with name `<state>`.
If there are required fields, the type must include field:
```c
command_file_RequiredFlags required_flags;
```

### `prefix`

Applied to all generated DTOs and handler names.

```
Move → my_Move_DispatchDto
```

### `debug`

If true, generates:

```
void debug_prefixCommand_DispatchDto(...)
```

### `default`

If true, generates a default handler invoked when no command matches.

Generated DTO:

```c
struct <commandSet>_default_DispatchDto {
    command_file_DispatchDto *parent;
    <State> *state;
    char *skip_label;
    const char *command;
};
```

### `defaultrequired`

If true, all commands are required unless overridden.

### `list`

If true, command strings match any name (use for list-style commands without explicit keywords).

### `fieldsetter`

If true, generates trivial setters such as:

```c
dto->state->CommandName = dto->field;
```

---

## Command Definition Format

Each command is declared as an object with fields:

```
<param>: <type>
validate:<param>: <validator>  (optional)
:required: true|false          (optional)
```



### Supported parameter types

| Type      | C Mapping      |
| --------- | -------------- |
| `"str"`   | `const char *` |
| `"int"`   | `int`          |
| `"float"` | `double`       |

### Validators

#### **Range** (`int` or `float`)

```json
"validate:x": { "type": "range", "min": 0, "max": 10 }
```

#### **Length** (`str`)

```json
"validate:name": { "type": "length", "min": 1, "max": 64 }
```

#### **Choice** (`int` or `str`)

```json
"validate:type": { "type": "choice", "options": ["A", "B"] }
```

---

## Adding Command Handlers

After generation, implement your command functions:

```c
void dispatch__handle_my_Move(struct my_Move_DispatchDto *dto)
{
    printf("Move to %d,%d\n", dto->x, dto->y);
}
```

---

## Integration With command_file.c

Use `read_command_file()`:

```c
read_command_file("script.txt", dispatch__MyCommands, &state);
```

Your JSON-defined handlers automatically process commands and parameters.

---

## Example Script File (command_file format)

```
# This is a comment

Move:"10" "20"
Say:"Hello world"

:label1
Move:"5" "5"
```

---

## Limitations

* Max 32 parameters per command (`command_file_PARAMS_MAX`)
* Max 256 chars per line (`command_file_LINE_MAX`)
* Validators must use one of the supported forms
* Required fields cap at 800 (warnings printed after)
