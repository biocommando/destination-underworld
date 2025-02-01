function bossToIni(fileData) {
const fs = require('fs')

let hasInitProperties = true
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

const game_modes = {
    normal: 0,
    brutal: GAMEMODIFIER_BRUTAL,
    over_powerup: GAMEMODIFIERS_OVER_POWERUP,
    explosion_madness: GAMEMODIFIER_DOUBLED_SHOTS,
    powerup_only: GAMEMODIFIER_MULTIPLIED_GOLD,
    potion_only: GAMEMODIFIER_POTION_ON_DEATH | GAMEMODIFIER_MULTIPLIED_GOLD | GAMEMODIFIER_NO_GOLD
}

const game_mode = key => {
    const arenaFlag = (key.startsWith('arena_') || game_mode.arena) ? GAMEMODIFIER_ARENA_FIGHT : 0
    return game_modes[key.replace('arena_', '')] | arenaFlag
}

const override_event = (name, gameMode) => `[overrides__${name}__for_mode_${game_mode(gameMode)}]`

const disable_event = (name, gameMode) => `on ${override_event(name, gameMode)} @never do @nothing`

const set_waypoint_sequence = seq => {
    const result = []
    for (let i = 0; i < seq.length; i++) {
        result.push(`on waypoint_reached: ${seq[i]} do set_waypoint: ${seq[(i + 1) % seq.length]}`)
    }
    return result
}

const intermediateBossFileForDebug = []
let lineProcessingDone = false

try {
    let data = fileData
    data.replace(/\{#([^#]*)#\}/gm, (whole, a) => {
            try {
                const result = eval(a)
                if (result.map)
                    return result.join('\n')
                return result
            } catch (e) {
                console.log('Error while executing code snippet:', whole)
                throw e
            }
        }).split(/\r?\n/)
        .map(x => x.trim())
        .filter(x => x[0] !== '/')
        .filter(x => !Number(x))
        .forEach(x => {
            // main:
            // set param = value
            // For overriding main settings for different game modes:
            // set [mode] param = value
            // events:
            // on <trigger_type>: <trigger_value> do <event_type>: <param_name> = <param_value>, ...
            // Events may have name:
            // on [EventName] <trigger_type>: ...
            // Set event properties for spawn and waypoint events:
            // set_event_property event_name: prop1 = val1, prop2 = val2, ...
            // preprocessor:
            // ms(..) = calculate boss timer value for amount of milliseconds
            // {#..#} = execute javascript
            x = x.replace(/ms\(([\d]+?)\)/g, (_, a) => ms(a))
            x = x.replace(/@([a-z_]*)/g, '$1: 0')
            intermediateBossFileForDebug.push(x)
            if (x.startsWith('on ')) {
                x = x.replace('on ', '')
                let evt = {}
                x = x.replace(/\[([a-zA-Z0-9_]*)\]/, (_, name) => {
                    evt.name = name
                    return ''
                })
                const trigger_type = x.split(':')[0].trim()
                let trigger_value = x.split(':')[1].split(' do ')[0].trim()
                if (trigger_type === 'waypoint_reached') {
                    let wp = waypoints.find(y => y.name === trigger_value)
                    const coords = x.split(' do ')[1].split(':')[1]
                    if (!wp && coords.split(';')[1]) {
                        wp = { name: coords.trim(), x: coords.split(';')[0].trim(), y: coords.split(';')[1].trim(), value: getNextId() }
                        waypoints.push(wp)
                    }
                    trigger_value = wp.value
                }
                const event_type = x.split(' do ')[1].split(':')[0].trim()
                evt = { ...evt, trigger_type, trigger_value, event_type }
                x = x.split(' do ')[1].split(':')[1]
                if (event_type === 'set_waypoint') {
                    let wp = waypoints.find(y => y.name === x.trim())
                    if (!wp && x.split(';')[1]) {
                        wp = { name: x.trim(), x: x.split(';')[0].trim(), y: x.split(';')[1].trim(), value: getNextId() }
                        waypoints.push(wp)
                    }
                    evt.x = wp.x
                    evt.y = wp.y
                    evt.waypoint_id = wp.value
                } else if (event_type === 'spawn') {
                    const params = x.split(',').map(y => y.trim())
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
                } else if (x && event_type !== 'nothing') {
                    const params = x.split(',').map(y => y.split('=').map(z => z.trim()))
                    params.forEach(p => {
                        evt[p[0]] = p[1]
                    })
                }
                events.push(evt)
            } else if (x.startsWith('set ')) {
                let mainName = 'main'
                x = x.replace(/\[([a-zA-Z0-9_]*)\]/, (_, name) => {
                    mainName = name
                    return ''
                })
                if (mainName !== 'main' && !mainSetup[mainName]) {
                    mainSetup[mainName] = { ...main }
                }
                x = x.replace('set ', '').split('=').map(y => y.trim())
                mainSetup[mainName][x[0]] = x[1]
            } else if (x.startsWith('set_event_property ')) {
                x = x.replace('set_event_property ', '')
                const prop = x.split(':')[1].split('=').map(z => z.trim())
                const evtName = x.split(':')[0]
                events.filter(e => e.name && (e.name === evtName || e.name.startsWith('overrides__' + evtName + '__for_mode_')))
                    .forEach(e => e[prop[0]] = prop[1])
            } else if (x === 'ignore_init_properties') {
                hasInitProperties = false
            }
        })
    lineProcessingDone = true

    const objToIni = (obj, ignoreKeys = []) => {
        return Object
            .keys(obj)
            .filter(key => !ignoreKeys.includes(key))
            .map(key => key + '=' + obj[key])
            .join('\r\n') + '\r\n'
    }

    let newFormat = []

    if (hasInitProperties) {
        Object.keys(mainSetup).forEach(name => {
            const gm = name === 'main' ? '' : game_mode(name)
            Object.keys(mainSetup[name])
                .forEach(key => newFormat.push(`${key} ${mainSetup[name][key]} ${gm}`))
        })
    }
    

    function eventToNewFormat(event) {
        if (event.name && event.name.startsWith('overrides__')) {
            newFormat.push(`event_override ${event.name.split('_').pop()}`)
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

    const isOverridingEvent = e => e.name && e.name.startsWith('overrides__')

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
        .map(e => {
            const mainevt = events.find(e2 => e2.name === e.name.replace(/overrides__(.*)__for_mode_\d*/, '$1'))
            if (!mainevt)
                throw 'base event not found for override ' + e.name
            if (e.trigger_type === 'inherit') {
                e.trigger_type = mainevt.trigger_type
                e.trigger_value = mainevt.trigger_value
            }
            mainevt['mode_override_' + e.name.split('__for_mode_')[1]] = e.name
            if (e.inheritAction) {
                const {name, trigger_type, trigger_value} = e
                for (prop in mainevt)
                    e[prop] = mainevt[prop]
                e.name = name
                e.trigger_type = trigger_type
                e.trigger_value = trigger_value
            }
            Object.keys(e).forEach(key => {
                if (key.startsWith('mode_override'))
                    delete e[key]
            })
        })

    events.filter(x => !x.name || !x.name.startsWith('overrides__')) // returns the "base" events, also those that have no overrides
    .forEach(evt => {
        eventToNewFormat(evt)
        // Find the events that override this event and serialize them right after so that
        // the overrides reference the correct base event
        events.filter(x => x.name && x.name.startsWith(`overrides__${evt.name}__for_mode_`))
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