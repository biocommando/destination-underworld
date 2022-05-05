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

const ms = a => Math.floor(Number(a) / 40 / 3)

const internal = {
    filename: `core-pack/${fname.replace('.boss', '.ini')}`
}

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
    js$ set_override_main_setup('arena_powerup_only') $
*/
const set_override_main_setup = key => `set mode_override_${game_mode(key)} = main__${key}`

const override_event = (name, gameMode) => `[overrides__${name}__for_mode_${game_mode(gameMode)}]`

const disable_event = (name, gameMode) => `on ${override_event(name, gameMode)} never: 0 do nothing: 0`

fs
    .readFileSync(fname)
    .toString().split(/\r?\n/)
    .map(x => x.trim())
    .filter(x => x[0] !== '/')
    .forEach(x => {
        // main:
        // set param = value
        // events:
        // on <trigger_type>: <trigger_value> do <event_type>: <param_name> = <param_value>, ...
        // Events may have name:
        // on [EventName] <trigger_type>: ...
        // waypoints:
        // def waypoint name = x, y
        // spawnpoints:
        // def spawnpoint name = x, y, probs
        // preprocessor:
        // ms(..) = calculate boss timer value for amount of milliseconds
        // js$..$ = execute javascript
        // internal parameters (e.g. filename):
        // set_internal param = value
        x = x.replace(/js\$([^$]+?)\$/g, (_, a) => {
            const execute = fn => {
                fn()
                return ''
            }
            return eval(a)
        })
        x = x.replace(/ms\(([\d]+?)\)/g, (_, a) => ms(a))
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
                // Defining spawnpoint on the fly
                if (x.trim().startsWith('(')) {
                    x = x.replace('(', '').replace(')', '')
                    const params = x.split(',').map(y => y.trim())
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
                        sp.name = sp2.name
                        return JSON.stringify(sp) === JSON.stringify(sp2)
                    })
                    if (existingSp) {
                        evt.spawn_point = existingSp.value
                    } else {
                        sp.value = getNextId()
                        delete sp.name
                        evt.spawn_point = sp.value
                        spawnpoints.push(sp)
                    }
                } else {
                    const sp = spawnpoints.find(y => y.name === x.trim())
                    if (!sp)
                        throw 'spawnpoint not found: ' + x
                    evt.spawn_point = sp.value
                }
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
            if (mainName !== 'main' && !mainSetup[mainName])
            {
                mainSetup[mainName] = {...main}
            }
            x = x.replace('set ', '').split('=').map(y => y.trim())
            mainSetup[mainName][x[0]] = x[1]
        } else if (x.startsWith('def waypoint')) {
            x = x.replace('def waypoint', '').split('=').map(y => y.trim())
            const params = x[1].split(',').map(y => y.trim())
            waypoints.push({ name: x[0], x: params[0], y: params[1], value: getNextId() })
        } else if (x.startsWith('def spawnpoint')) {
            x = x.replace('def spawnpoint', '').split('=').map(y => y.trim())
            const params = x[1].split(',').map(y => y.trim())
            spawnpoints.push({
                name: x[0], x: params[0], y: params[1],
                enemy_0_probability: params[2],
                enemy_1_probability: params[3],
                enemy_2_probability: params[4],
                enemy_3_probability: params[5],
                enemy_4_probability: params[6],
                value: getNextId()
            })
        } else if (x.startsWith('set_internal ')) {
            x = x.replace('set_internal ', '').split('=').map(y => y.trim())
            internal[x[0]] = x[1]
        } else if (x.startsWith('set_event_property ')) {
            x = x.replace('set_event_property ', '')
            const prop = x.split(':')[1].split('=').map(z => z.trim())
            const evtName = x.split(':')[0]
            events.filter(e => e.name && (e.name === evtName || e.name.startsWith('overrides__' + evtName + '__for_mode_')))
                .forEach(e => e[prop[0]] = prop[1])
        }
    })

const objToIni = (obj, ignoreKeys = []) => {
    return Object.keys(obj).filter(key => !ignoreKeys.includes(key)).map(key => key + '=' + obj[key]).join('\r\n') + '\r\n'
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

fs.writeFileSync(internal.filename, str)

