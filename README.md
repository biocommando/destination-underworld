This is a game called Destination Underworld for Windows.
It's a 2D top-down shooter game I've been working on since 2011.
The core mechanics (and most of the levels) have remained untouched
for a very long time and I've been just adding features, tidying up
the code and finetuning the difficulty level.

# Development Enviroment

The game can be built and run on either Windows or Linux. On Linux the
allegro5 libraries need to be installed via package manager (see Allegro
documentation). The text onwards talks about Windows build but everything
on top level applies to Linux as well.

The game requires GCC (MinGW) for compiling. The Windows binaries for the
game programming library [allegro5](https://liballeg.org) need to be
installed on the computer. By default, it's assumed that the binary package
is located in `.\allegro` in the project root directory but the location
can be modified in `bake\environment-common.js`. The build process and level editor
also use NodeJS scripts so `node` needs to be also installed.

So, the list of dependencies in short:
- GCC / MinGW
- Allegro 5.2
- NodeJS

## Building

The project uses its own `Bake` build system that is basically a simple NodeJS based
task sequencer. You can build a debug build using simply `node bake.js` which is
equivalent to `node bake.js debug`. The debug build creates the game executable
`DestinationUnderworld.exe` and test programs `unittests.exe` and `test-du.exe`.

Other targets are listed in `bake.json`. The most commonly used are:
- `release`: Creates a release build and copies all the required 
  files to run the game under `.\DestinationUnderworldRelease` directory.
  Uses the `environment-release.js` Bake script that assumes that you define your
  own `auth_setup` configuration that setups a custom `DMAC_SHA1_KEY_FN` function.
  This function should copy a 64 byte DMAC key to the `char *key` argument, selected
  based on the `int ctx` which can be 0..3. A very simplistic implementation would be
  `void dmac_sha1_default_key_fn(char *key, int ctx) { memset(key, 0, 64); sprintf(key, "<a random string here>%c", ctx); }`
  If you don't care about authenticating data files and recordings, you can just use a dummy
  script like `module.exports = ()=>{}`.
- `test`: Run unit and play tests
- `unit-tests`: Compile unit tests
- `dispatchers`: Generate command file dispatcher code from `du_commands.json`

## Level editor

Run the level editor using the `run-level-editor.bat` under the root directory.
If you want to edit something else than the levels from the default level pack
("core-pack"), you'll need to pass the level pack name to the batch file using the
syntax: `run-level-editor.bat --mission-pack NAME`, example:
`.\run-level-editor.bat --mission-pack robot-uprising`. You can also change the
level pack from the UI.

The level editor UI is a bit arcane and especially the more advanced level editor
stuff like conditional items or scripted events can be difficult to understand.
I'll try to find some time to document the editor at some point.

# License

## Program and data

Copyright 2011-2025 Joonas Salonpää

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

## Images and sounds

Player and enemy sprites based on templates by Charles Gabriel:
https://opengameart.org/content/twelve-16x18-rpg-sprites-plus-base
License CC-BY 3.0 (https://creativecommons.org/licenses/by/3.0/)

Other art, including music and soundeffects, are created by me,
Joonas Salonpää, under license CC-BY 3.0 (see link above), with the exception
of the level end screen background (the green wizard surrounded by skeletons) which
is created using AI and has no copyright.