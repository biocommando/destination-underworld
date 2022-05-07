const fs = require('fs')

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

const fname = process.argv.find(x => x.endsWith('.boss'))

const ms_delta = 40 * 3
const ms = a => Math.floor(Number(a) / ms_delta)

let output_filename = `core-pack/${fname.replace('.boss', '.ini')}`

const GAMEMODIFIER_DOUBLED_SHOTS = 0x1
const GAMEMODIFIER_OVERPOWERED_POWERUPS = 0x2
const GAMEMODIFIER_OVERPRICED_POWERUPS = 0x4
const GAMEMODIFIERS_OVER_POWERUP = (GAMEMODIFIER_OVERPOWERED_POWERUPS | GAMEMODIFIER_OVERPRICED_POWERUPS)
const GAMEMODIFIER_MULTIPLIED_GOLD = 0x8
const GAMEMODIFIER_BRUTAL = 0x10
const GAMEMODIFIER_ARENA_FIGHT = 0x20

const game_modes = {
    normal: 0,
    brutal: GAMEMODIFIER_BRUTAL,
    over_powerup: GAMEMODIFIERS_OVER_POWERUP,
    explosion_madness: GAMEMODIFIER_DOUBLED_SHOTS,
    powerup_only: GAMEMODIFIER_MULTIPLIED_GOLD
}

const game_mode = key => {
    const arenaFlag = (key.startsWith('arena_') || game_mode.arena) ? GAMEMODIFIER_ARENA_FIGHT : 0
    return game_modes[key.replace('arena_', '')] | arenaFlag
}
/*
    Configuration example using this macro helper:
    set player_initial_gold = 6
    set [main__powerup_only] player_initial_gold = -1
    {# set_override_main_setup('arena_powerup_only') #}
*/
const set_override_main_setup = key => `set mode_override_${game_mode(key)} = main__${key}`

const override_event = (name, gameMode) => `[overrides__${name}__for_mode_${game_mode(gameMode)}]`

const disable_event = (name, gameMode) => `on ${override_event(name, gameMode)} never: 0 do nothing: 0`

const set_waypoint_sequence = seq => {
    const result = []
    for (let i = 0; i < seq.length; i++) {
        result.push(`on waypoint_reached: ${seq[i]} do set_waypoint: ${seq[(i + 1) % seq.length]}`)
    }
    return result
}

const script_execute = fn => {
    fn()
    return ''
}

const intermediateBossFileForDebug = []
let lineProcessingDone = false

try {
    fs
        .readFileSync(fname)
        .toString().replace(/\{#([^#]*)#\}/gm, (whole, a) => {
            const execute = script_execute
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
            // set [override_config_name] param = value
            // These overrides must be taken into use by setting mode_override_X
            // property to <override_config_name> for the main settings (X = mode code)
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
            x = x.replace(/@inherit/g, 'inherit: 0')
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

                    if (!wp && x.split(';')[1]) {
                        wp = { name: x.trim(), x: x.split(';')[0].trim(), y: x.split(';')[1].trim(), value: getNextId() }
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

    let str = `# Generated from ${fname}\r\n`

    main.events = events.filter(e => !(e.name && e.name.startsWith('overrides__'))).length

    Object.keys(mainSetup).forEach(sect => {
        str += `\r\n[${sect}]\r\n${objToIni(mainSetup[sect])}`
    })

    events.forEach(e => {
        if (e.event_id !== undefined) {
            const name = e.event_id
            e.event_id = events.filter(e => !(e.name && e.name.startsWith('overrides__'))).findIndex(x => x.name === name)
            if (e.event_id === -1)
                throw 'event not found with name ' + name
        }
    })

    events.filter(e => e.name && e.name.startsWith('overrides__'))
        .forEach(e => {
            const mainevt = events.find(e2 => e2.name === e.name.replace(/overrides__(.*)__for_mode_\d*/, '$1'))
            if (!mainevt)
                throw 'base event not found for override ' + e.name
            if (e.trigger_type === 'inherit') {
                e.trigger_type = mainevt.trigger_type
                e.trigger_value = mainevt.trigger_value
            }
            mainevt['mode_override_' + e.name.split('__for_mode_')[1]] = e.name
            if (e.inheritAction) {
                e = { ...mainevt, name: e.name, trigger_type: e.trigger_type, trigger_value: e.trigger_value }
            }
            Object.keys(e).forEach(key => {
                if (key.startsWith('mode_override'))
                    delete e[key]
            })
            str += `\r\n[${e.name}]\r\n${objToIni(e, ['name'])}`
        })

    events.filter(e => !(e.name && e.name.startsWith('overrides__'))).forEach((e, i) => {
        str += `\r\n[event_${i}]\r\n`
        str += objToIni(e, ['name'])
    })

    spawnpoints.forEach((s, i) => {
        str += `\r\n[spawn_point_${s.value}]\r\n`
        str += objToIni(s, ['name', 'value'])
    })

    fs.writeFileSync(output_filename, str)
} catch (e) {
    if (intermediateBossFileForDebug.length === 0)
        console.log('Error while running preprocessor:', e)
    else if (lineProcessingDone)
        console.log('Error while compiling to INI format:', e)
    else
        console.log('Error while processing line "%s":',
            intermediateBossFileForDebug[intermediateBossFileForDebug.length - 1], e)
} finally {
    fs.writeFileSync('intermediate_debug.boss', intermediateBossFileForDebug.join('\r\n'))
}