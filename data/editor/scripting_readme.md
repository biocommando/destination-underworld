It's possible to create scripted events to every room in the game.
The scripts are room-specific. The scripting was originally created
just for boss fights but has been extended to be more generic.

The scripts used to be implemented in a custom language that would be
transpiled to the final format but the transpilation was replaced with
a javascript API for better flexibility, and to make the scripting engine
a bit less magical.

For more documentation on the actions and trigger conditions, see bossfightconf.h.

# Javascript API

The format that the game itself reads isn't very easy to program directly
so there's a configuration interface that uses a custom Javascript API.
The API has 3 kinds of commands:
- event definition
- set game initialization property
- set event property

## Event definitions
Events are defined as objects that have the following fields:
- name (optional, for event overloading)
- trigger_type
- trigger_value
- event_type
- event_value

The events must be passed to `processEvent` function to make them a part of the final export.

There's a helper function `e` for creating event definitions that has the
following syntax:
```js
e.on(trigger_type).is(trigger_value).do(event_type).to(event_value)
// or named events:
e(name).on(trigger_type).is(trigger_value).do(event_type).to(event_value)
```

The API has defined all the trigger and event types as constants, so you don't need
to use quotes for the parameters.

### Trigger types and values
(if nothing is stated about the value, it's just used as is):

- waypoint_reached, value: `x;y`. The x, y coordinate identifies the waypoint. Example:
  ```js
  e.on(waypoint_reached).is('6;5').do('disallow_firing').to('')
  ```
	* Triggers when the boss reaches this coordinate in waypoint mode
- time_interval
	* Triggers when primary timer modulo this value (number of frames / 3) is 0
- time_one_time
	* Triggers when primary timer reaches this value (number of frames / 3)
- health
	* Triggers when boss health goes to or below this level
- secondary_timer
	* Triggers when secondary timer reaches this value (initially not started)
- kill_count
	* Triggers when this many enemies have been killed in this room
- positional_trigger
	* Triggers from "positional trigger" objects in the level

### Event types and values.
If value is said to be key-value list, it means that it has format 'key = value, key = value, ...'
and only the keys are listed:

- set_waypoint, value similar to waypoint_reached
	* Set the next coordinate where the boss tries to walk to

- spawn, value `x, y, prob1, prob2, prob3, prob4, prob5`; contains spawn coordinates and probabilities for
each enemy type (should sum up to 100).

- fire_in_circle, keys: number_of_directions, intensity
	* Trigger a cluster explosion at the boss coordinates

- modify_terrain, keys: x, y, terrain_type
	* types: floor, wall, level_exit

- start_secondary_timer, keys: time

- toggle_event_enabled, keys: event_id (= event's name)

- spawn_potion: keys: x, y, type

- allow_firing
	* Allow boss to shoot the player

- disallow_firing
	* Don't allow boss to shoot the player

- clear_waypoint
	* Change the movement mode back to "normal movement"

- stop_secondary_timer

**Special trigger types:**
- never = event not triggered unless overridden with valid trigger
- inherit = event trigger inherited when overridden

**Special event actions:**
- nothing = doesn't do anything. Should be used with "never" trigger type.
- inherit = event action inherited when overridden

Special event/trigger types don't use the value field for anything. To
omit the value (and make the syntax less mind boggling) you can use the special
methods of the `e` event builder function, like:
```js
e('evt_mod_terrain_9').never().do_nothing()
// Inheriting
e(override_event('evt_mod_terrain_9', 'brutal')).inherit().do(modify_terrain).to('x = 1, y = 1, terrain_type = wall')
e(override_event('evt_mod_terrain_9', 'brutal')).on(kill_count).is(10).inherit()
```

Events can be overridden but it can be only done using the javascript preprocessing
functions (see below).

## Setting game initialization property

The following initialization properties are understood:
- health = boss health
	* Needs to be set in the same room as where the boss is
- speed = boss speed
	* Needs to be set in the same room as where the boss is
- fire_rate = boss fire rate
	* Needs to be set in the same room as where the boss is
- player_initial_gold = override player's initial gold amount
	* Needs to be set in the first room
- time_starts_at = start primary timer with this value
	* Room specific

Set the properties using syntax:
```js
set(property, value)
```

Override the property for certain mode:
```js
set(property, value, mode)
// Example
set('player_initial_gold', 10, 'powerup_only')
```

## Setting event property

Set a generic property on a named event.
Currently only one property: initially_disabled (=can be toggled to be on)
Syntax:
```js
set_event_property(name, 'initially_disabled', is_disabled ? 1 : 0)
```

# Helper functions

For the javascript execution there are a couple of convenience functions/variables available:
- `ms(..)` = calculate boss timer value for amount of milliseconds
- `override_event(base_event, game_mode_name)`
	* Creates a mapping that maps the base event to this event if the game mode flags are specified.
	Example:
	```js
	e(override_event('SpawnA1', 'brutal').on(time_interval).is(ms(_spawn_interval * 1.5)).inherit()
	```
- `disable_event(name, game_mode_name)`
	* Disables an event for certain mode
- `set_waypoint_sequence(sequence)`
	* Sets boss a route with waypoints. E.g. boss walking between two points:
	```js
	set_waypoint_sequence([ '3;3', '9;9' ])
	```
- `ms_delta`
	* Boss timer tick in milliseconds
- `implicit_arena_game_mode()`
	* Calling this function sets arena flag to 1 so that you don't need to refer to the arena variant of the
	game mode when overriding events / properties, so instead of having to write
	`set('health', 20, 'arena_brutal')` you can write:
	```
		implicit_arena_game_mode()
		set('health', 20, 'brutal')
	```
- `modify_rect(trig_type, trig_value, x, y, x2, y2, terrain_type)`
	* Creates events with `on(trig_type).is(trig_value)` as the trigger for changing all tiles in a rectangular area to the provided terrain type
	* Example: after killing 10 enemies, change 2x3 area at x=5, y=7 to floor:
	```js
		modify_rect('kill_count', 10, 5, 7, 7, 10, 'floor')
	```

# Game modes

The following names are used for game modes when using the override syntax:
```
normal
brutal
over_powerup
explosion_madness
powerup_only
potion_only
```

For arena fights the game mode names are prefixed with `arena_` if `implicit_arena_game_mode()` is not called.