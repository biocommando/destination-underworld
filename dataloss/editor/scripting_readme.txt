It's possible to create scripted events to every room in the game.
The scripts are room-specific. The scripting was originally created
just for boss fights but has been extended to be more generic.

The format that the game itself reads isn't very easy to program directly
so there's a transpiler that uses a custom language (BOSS). The language has
4 kinds of commands:
- event definition
- set game initialization property
- set event property
- ignore default game initialization properties

Event definitions have the following format:
on [event_name] trigger_type: trigger_value do action_type: parameters

The event name is optional.

Trigger types and values (if nothing is stated about the value, it's just used as is):

waypoint_reached, value: "x;y". The x, y coordinate identifies the waypoint. Example:
on waypoint_reached: 6;5 do disallow_firing

Action types and values. If value is said to be key-value list, it means that it has format 'key = value, key = value, ...'
and only the keys are listed:

set_waypoint, value similar to waypoint_reached

spawn, value "x, y, prob1, prob2, prob3, prob4, prob5"; contains spawn coordinates and probabilities for
each enemy type (should sum up to 100).

fire_in_circle, keys: number_of_directions, intensity

modify_terrain, keys: x, y, terrain_type

start_secondary_timer, keys: time

toggle_event_enabled, keys: event_id (= event's name)

spawn_potion: keys: x, y, type

Special trigger types:
never = event not triggered unless overridden with valid trigger
inherit = event trigger inherited when overridden
Special event actions:
nothing = doesn't do anything. Should be used with "never" trigger type.
inherit = event action inherited when overridden

Special event/trigger types don't use the value field for anything. To
omit the value (and make the syntax less mind boggling) prefix the special
types with '@'. Example:
on [evt_mod_terrain_9] @never do @nothing

Events can be overridden but it can be only done using the javascript preprocessing
functions (see below).

Setting game initialization property:

The following initialization properties are understood:
health = boss health
speed = boss speed
fire_rate = boss fire rate
player_initial_gold = override player's initial gold amount
time_starts_at = start primary timer with this value

Set the properties using syntax:
set property = value

Override the property for certain mode:
set [mode] property = value

Setting event property:

Set a generic property on a named event.
Currently only one property: initially_disabled (=can be toggled to be on)
Syntax:
set_event_property name: initially_disabled = 1 or 0

Ignoring default game initialization properties:

If the level is not a boss level, you should add this line to the code:
ignore_init_properties
This way the default properties won't be added to the resulting script file.

Preprocessing:

{#..#} = execute javascript and replace the snippet with the returned string or list of strings (=lines).
ms(..) = calculate boss timer value for amount of milliseconds

Preprocessing is run in this order so you can use {# .. #} inside ms( .. )

For the javascript execution there are a couple of convenience functions/variables available:
override_event(base_event, game_mode_name)
	Creates a mapping that maps the base event to this event if the game mode flags are specified.
	Example:
	on {# override_event('SpawnA1', 'brutal') #} time_interval: ms({# _spawn_interval * 1.5 #}) do @inherit
disable_event(name, game_mode_name)
	Disables an event for certain mode
set_waypoint_sequence(sequence)
	Sets boss a route with waypoints. E.g. boss walking between two points:
	{# set_waypoint_sequence([ '3;3', '9:9' ]) #}
ms_delta
	Boss timer tick in milliseconds
implicit_arena_game_mode()
	Calling this function sets arena flag to 1 so that you don't need to refer to the arena variant of the
	game mode when overriding events / properties, so instead of having to write
	"set [arena_brutal] health = 20" you can write:
		{# implicit_arena_game_mode() #}
		set [brutal] health = 20

You can add line comments by prefixing any line with a single slash ('/'). Of course, also double slashes
are valid.