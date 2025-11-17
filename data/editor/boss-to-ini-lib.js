function bossToIni(fileData) {
const fs = require('fs')

let hasInitProperties = false
const main = {
    health: 50,
    fire_rate: 10,
    speed: 1,
    player_initial_gold: -1
}

const mainSetup = {
    main
}

const events = []

const spawnpoints = []

const waypoints = []

let idCounter = 0
const getNextId = () => ++idCounter

const ms_delta = 40 * 3
const ms = a => Math.floor(Number(a) / ms_delta)

const GAMEMODIFIER_DOUBLED_SHOTS = 0x1
const GAMEMODIFIER_OVERPOWERED_POWERUPS = 0x2
const GAMEMODIFIER_OVERPRICED_POWERUPS = 0x4
const GAMEMODIFIERS_OVER_POWERUP = (GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS)
const GAMEMODIFIER_MULTIPLIED_GOLD = 0x8
const GAMEMODIFIER_BRUTAL = 0x10
const GAMEMODIFIER_ARENA_FIGHT = 0x20
const GAMEMODIFIER_POTION_ON_DEATH = 0x40
const GAMEMODIFIER_NO_GOLD = 0x80
const GAMEMODIFIER_UBER_WIZARD = 0x100

const game_modes = {
    normal: 0,
    brutal: GAMEMODIFIER_BRUTAL,
    over_powerup: GAMEMODIFIERS_OVER_POWERUP,
    explosion_madness: GAMEMODIFIER_DOUBLED_SHOTS,
    powerup_only: GAMEMODIFIER_MULTIPLIED_GOLD,
    potion_only: GAMEMODIFIER_POTION_ON_DEATH | GAMEMODIFIER_MULTIPLIED_GOLD | GAMEMODIFIER_NO_GOLD,
    uber_wizard: GAMEMODIFIER_UBER_WIZARD | GAMEMODIFIER_NO_GOLD | GAMEMODIFIER_MULTIPLIED_GOLD
}

const game_mode = key => {
    const arenaFlag = (key.startsWith('arena_') || game_mode.arena) ? GAMEMODIFIER_ARENA_FIGHT : 0
    return game_modes[key.replace('arena_', '')] | arenaFlag
}

const overridingEvents = []

const override_event = (name, gameMode) => {
    const newName = `overrides__${name}__for_mode_${gameMode}`
    overridingEvents.push({base: name, gameMode: game_mode(gameMode), name: newName})
    return newName
}

const _override_event = (name, gameMode) => {
    return `[${override_event(name, gameMode)}]`
}

const getOverrideDetails = e => overridingEvents.find(x => x.name === e.name)
const isOverridingEvent = e => !!getOverrideDetails(e)
const getBaseEvent = e => isOverridingEvent(e) ? getOverrideDetails(e).base : undefined
const getOverridingEventMode = e => isOverridingEvent(e) ? getOverrideDetails(e).gameMode : undefined

const disable_event = (name, gameMode) => e(override_event(name, gameMode)).never().do_nothing()

const set_waypoint_sequence = seq => {
    const result = []
    for (let i = 0; i < seq.length; i++) {
        //result.push(`on waypoint_reached: ${seq[i]} do set_waypoint: ${seq[(i + 1) % seq.length]}`)
        e.on('waypoint_reached').is(seq[i]).do('set_waypoint').to(seq[(i + 1) % seq.length])
    }
    return result
}

const implicit_arena_game_mode = () => game_mode.arena = 1

const modify_rect = (trigtype, trigvalue, x, y, x2, y2, terrain_type) => {
    const result = []
    for (let _x = x; _x <= x2; _x++)
    {
        for (let _y = y; _y <= y2; _y++)
        {
            //result.push(`on ${trig} do modify_terrain: x = ${_x}, y = ${_y}, terrain_type = ${terrain_type}`)
            e.on(trigtype).is(trigvalue).do('modify_terrain').to(`x = ${_x}, y = ${_y}, terrain_type = ${terrain_type}`)
        }
    }
    return result
}

const intermediateBossFileForDebug = []
let lineProcessingDone = false

function processEvent(event) {
    let evt = {}
    evt.name = event.name
    const trigger_type = event.trigger_type
    let trigger_value = event.trigger_value
    if (trigger_type === 'waypoint_reached') {
        let wp = waypoints.find(y => y.name === trigger_value)
        const coords = trigger_value
        if (!wp && coords.split(';')[1]) {
            wp = { name: coords.trim(), x: coords.split(';')[0].trim(), y: coords.split(';')[1].trim(), value: getNextId() }
            waypoints.push(wp)
        }
        trigger_value = wp.value
    }
    const event_type = event.event_type
    evt = { ...evt, trigger_type, trigger_value, event_type }
    let event_value = event.event_value
    if (event_type === 'set_waypoint') {
        let wp = waypoints.find(y => y.name === event_value.trim())
        if (!wp && event_value.split(';')[1]) {
            wp = { name: event_value.trim(), x: event_value.split(';')[0].trim(), y: event_value.split(';')[1].trim(), value: getNextId() }
            waypoints.push(wp)
        }
        evt.x = wp.x
        evt.y = wp.y
        evt.waypoint_id = wp.value
    } else if (event_type === 'spawn') {
        const params = event_value.split(',').map(y => y.trim())
        if (params.length !== 7 || params.some(isNaN)) throw 'Unable to parse spawn parameters'
        const sp = {
            x: params[0], y: params[1],
            enemy_0_probability: params[2],
            enemy_1_probability: params[3],
            enemy_2_probability: params[4],
            enemy_3_probability: params[5],
            enemy_4_probability: params[6]
        }
        const existingSp = spawnpoints.find(sp2 => {
            sp.value = sp2.value
            return JSON.stringify(sp) === JSON.stringify(sp2)
        })
        if (existingSp) {
            evt.spawn_point = existingSp.value
        } else {
            sp.value = getNextId()
            evt.spawn_point = sp.value
            spawnpoints.push(sp)
        }
    } else if (event_type === 'inherit') {
        evt.inheritAction = true
    } else if (event_value && event_type !== 'nothing') {
        const params = event_value.split(',').map(y => y.split('=').map(z => z.trim()))
        params.forEach(p => {
            evt[p[0]] = p[1]
        })
    }
    events.push(evt)
}
function e(name) {

	const o = {
		name,
		trigger_type: function (v) { this.trigger_type = v; return this },
		trigger_value: function (v) { this.trigger_value = v; return this },
		event_type: function (v) { this.event_type = v; return this },
		event_value: function (v) { this.event_value = v; processEvent(this); return this },
	}
	o.on = o.trigger_type
	o.is = o.trigger_value
	o.do = o.event_type
	o.to = o.event_value

	o.inherit = () => {
		if (typeof(o.trigger_type) === 'function')
			return o.on('inherit').is(0)
		return o.do('inherit').to(0)
	}
	o.never = () => o.on('never').is(0)
	o.do_nothing = () => o.do('nothing').to(0)
	return o
}

e.on = v => e().on(v)

function set(variable, value, mainName = 'main') {
    hasInitProperties = true
    if (mainName !== 'main' && !mainSetup[mainName]) {
        mainSetup[mainName] = { ...main }
    }
    mainSetup[mainName][variable] = value
}

function set_event_property(event_name, property_name, property_value) {
    events.filter(e => e.name === event_name || getBaseEvent(e) === event_name)
        .forEach(e => e[property_name] = property_value)
}

try {
    let data = fileData

    {
        const time_interval = 'time_interval', time_one_time = 'time_one_time',
            health = 'health', waypoint_reached = 'waypoint_reached', secondary_timer = 'secondary_timer',
            kill_count = 'kill_count', positional_trigger = 'positional_trigger',
            spawn = 'spawn', allow_firing = 'allow_firing', disallow_firing = 'disallow_firing',
            fire_in_circle = 'fire_in_circle', modify_terrain = 'modify_terrain', set_waypoint = 'set_waypoint',
            clear_waypoint = 'clear_waypoint', start_secondary_timer = 'start_secondary_timer',
            stop_secondary_timer = 'stop_secondary_timer', toggle_event_enabled = 'toggle_event_enabled',
            spawn_potion = 'spawn_potion'
        eval(data)
    }

    lineProcessingDone = true

    let newFormat = []

    if (hasInitProperties) {
        Object.keys(mainSetup).forEach(name => {
            const gm = name === 'main' ? '' : game_mode(name)
            Object.keys(mainSetup[name])
                .forEach(key => newFormat.push(`${key} ${mainSetup[name][key]} ${gm}`))
        })
    }
    

    function eventToNewFormat(event) {
        if (isOverridingEvent(event)) {
            newFormat.push(`event_override ${getOverridingEventMode(event)}`)
        } else {
            newFormat.push(`event`)
        }
        if (event.trigger_type !== 'never') {
            newFormat.push(`event_trigger ${event.trigger_type} ${event.trigger_value}`)
        }
        if (event.event_type !== 'nothing') {
            let action = `event_action ${event.event_type} `
            if (event.event_type === 'spawn') {
                const sp = spawnpoints.find(x => x.value === event.spawn_point)
                for (let i = 0; i < 5; i++)
                    action += `${sp['enemy_' + i + '_probability']} `
                action += `${sp.x} ${sp.y}`
            }
            if (event.event_type === 'fire_in_circle') {
                action += `${event.number_of_directions} ${event.intensity}`
            }
            if (event.event_type === 'modify_terrain') {
                action += `${event.x} ${event.y} 0 0 0 0 0 ${event.terrain_type}`
            }
            if (event.event_type === 'set_waypoint') {
                action += `${event.x} ${event.y} ${event.waypoint_id}`
            }
            if (event.event_type === 'start_secondary_timer') {
                action += `${event.time}`
            }
            if (event.event_type === 'toggle_event_enabled') {
                action += `${event.event_id} ${event.enabled}`
            }
            if (event.event_type === 'spawn_potion') {
                action += `${event.x} ${event.y} ${event.type}`
            }
            newFormat.push(action)
        }
        if (event.initially_disabled) {
            newFormat.push('event_initially_disabled')
        }
    }

    // Check that each referenced event id actually exists
    events.forEach(e => {
        if (e.event_id !== undefined) {
            const name = e.event_id
            e.event_id = events.filter(e => !isOverridingEvent(e)).findIndex(x => x.name === name)
            if (e.event_id === -1)
                throw 'event not found with name ' + name
        }
    })

    // This logic handles the inherit logic
    events.filter(e => isOverridingEvent(e))
        .forEach(e => {
            const mainevt = events.find(e2 => e2.name === getBaseEvent(e))
            if (!mainevt)
                throw 'base event not found for override ' + e.name
            if (e.trigger_type === 'inherit') {
                e.trigger_type = mainevt.trigger_type
                e.trigger_value = mainevt.trigger_value
            }
            if (e.inheritAction) {
                const {name, trigger_type, trigger_value} = e
                for (prop in mainevt)
                    e[prop] = mainevt[prop]
                e.name = name
                e.trigger_type = trigger_type
                e.trigger_value = trigger_value
            }
        })

    events.filter(x => !isOverridingEvent(x)) // returns the "base" events, also those that have no overrides
    .forEach(evt => {
        eventToNewFormat(evt)
        // Find the events that override this event and serialize them right after so that
        // the overrides reference the correct base event
        // (note: getBaseEvent returns undefined if event is not overriding event and evt.name
        // is undefined if not defined so can't just check that these two are equal)
        events.filter(x => isOverridingEvent(x) && getBaseEvent(x) === evt.name)
            .forEach(eventToNewFormat)
    })
    newFormat.push('end')

    return newFormat.join('\n')
} catch (e) {
    let err = ''
    if (intermediateBossFileForDebug.length === 0)
        err = 'Error while running preprocessor:'
    else if (lineProcessingDone)
        err = 'Error while compiling to INI format:'
    else
        err = `Error while processing line "${intermediateBossFileForDebug[intermediateBossFileForDebug.length - 1]}":`
    console.log(err, e)
    return `>>ERROR>> ${err} ${e}`
} finally {
    fs.writeFileSync('intermediate_debug.boss', intermediateBossFileForDebug.join('\r\n'))
}
}

module.exports = { bossToIni }