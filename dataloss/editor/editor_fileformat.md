The file format that the levels use is documented in source code (read_level).
The editor expects the files to be in a certain format so that it can edit and visualize
the scripting and conditional objects on map.

The file begins with line "X(duedit_compatible)" where 'X' is the level format version and the rest of
the line hints the editor that it should be able to read the data.

The file contains different sections that are handled differently:

## tiles

Start marked by duscript label `*@tiles`. Objects that are active globally are defined here.

## conditions

Start marked by comment `#conditions`. If the line contains a duscript if statement, the goto label
is used as the condition name for the objects that are defined between the condition and the condition
skip label. Example:
```
#conditions
*?game_modifiers = 232 COND_potion
*>+SKIP_COND_potion
*@COND_potion
305 7 6 1
*@+SKIP_COND_potion
```
Here the object definition `305 7 6 1` is marked as having condition named `COND_potion`. The corresponding
representation in editor's metadata field is `condition: COND_potion: game_modifiers = 232` is saved to the
editor state. The condition names must begin with `COND_`.

## metadata:

Start marked by duscript label `*@metadata`. Each duscript variable definition is added to metadata.
The variable definitions are modified to a more human-readable format (e.g. `a"1"` would become `a = "1"`).

Supported metadata entries:
- name = Level name displayed in the info screen and when starting the level
- wall_color = RGB definition for top-most wall layer (example: "0.5 1 0.8")
- storyX (where X = number in range 0-10) = Story lines displayed after level (max length 60 characters)
- no_more_levels = This level is final level, no more levels will be loaded
- mute_bosstalk = Don't play the sample associated to the boss if the level has scripting definitions
- story_image = A custom image displayed after level (file name)

## compiled_script:

The script definitions read by the game. Start indicated by reading line in format `$NUMBER`.
The compiled script is basically just discarded when reading a level as it's re-compiled every time
the file is saved.

## script:

The script definitions in the BOSS script format (see scripting_readme.md). Start indicated by comment
line `#script ROOM_NUMBER` and the script snippet ends with `#end_script`. Each script line is converted
to a comment line so that the game logic doesn't get messed up by the editor-added metadata.