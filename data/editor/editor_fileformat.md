The file format that the levels use is "command file" format, documented in src/command_file.
The editor expects the files to be in a certain format so that it can edit and visualize
the scripting and conditional objects on map.

The file begins with line `header: "3" "(duedit_compatible)"` where '3' is the level format version and the rest of
the line hints the editor that it should be able to read the data.

The file contains different sections that are handled differently:

## tiles

Start marked by command file label `:tiles`. Objects that are active globally are defined here.
Object syntax is: `object: "id" "tile x coord" "tile y coord" "room id"` (room id in range 1..8).

## conditions

Start marked by comment `#conditions`. If the line contains a `condition` command, the first parameter is
interpreted as the condition (syntax: `<variable> <operator> <value> (and <variable> <operator> <value>...)`,
where variable is the predefined variable name, operator `=` for "is equal" or `!` for is not equal and value
base 10 decimal number; conditions can be chained using `and` operator) and the second parameter is a goto
label where the script branches (only forward) if the condition is met. The goto label is also
used as the condition name for the objects that are defined between the condition and the condition
skip label. Example:
```
#conditions
condition: "game_modifiers = 232" "COND_potion"
goto: "+SKIP_COND_potion"
:COND_potion
object: "305" "7" "6" "1"
:+SKIP_COND_potion
```
Here the object definition `object: "305" "7" "6" "1"` is marked as having condition named `COND_potion`. The corresponding
representation in editor's metadata field is `condition: COND_potion: game_modifiers = 232` is saved to the
editor state. The condition names must begin with `COND_` (required by the editor, not the game).

## metadata:

Start marked by command file label `:metadata`. Each variable definition is added to metadata.
Setting the variables is done using command syntax: `set_var: "name" "value"`.
The variable definitions are modified to a more human-readable format in the editor (e.g. `set_var: "a" "1"` would
become `a = "1"`).

Supported metadata entries:
- name = Level name displayed in the info screen and when starting the level
- wall_color = RGB definition for top-most wall layer (example: "0.5 1 0.8")
- storyX (where X = number in range 0-10) = Story lines displayed after level (max length 60 characters)
- mute_bosstalk = Don't play the sample associated to the boss if the level has scripting definitions
- story_image = A custom image displayed after level (file name)

## compiled_script:

The script definitions read by the game. Start indicated by reading line in format `script_start: "NUMBER"`
where the number is the room number in range 1..8. The script lines use the syntax `script_line: "script commands"`
The compiled script is basically just discarded when the editor reads a level as it's re-compiled every time
the file is saved.

## script:

The script definitions in the BOSS script format (see scripting_readme.md). Start indicated by comment
line `#script NUMBER` where the number is the room number in range 1..8 and the script snippet ends
with `#end_script`. Each script line is converted to a comment line so that the game logic doesn't get
messed up by the editor-added metadata.
