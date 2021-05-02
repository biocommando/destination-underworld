const fs = require('fs')

const main = {
    health: 50,
    fire_rate: 10,
    speed: 1,
    player_initial_gold: -1
}

const events = []

const spawnpoints = []

const waypoints = []

let idCounter = 0
const getNextId = () => ++idCounter

const fname = process.argv.find(x => x.endsWith('.boss'))

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
        // waypoints:
        // def waypoint name = x, y
        // spawnpoints:
        // def spawnpoint name = x, y, probs
        // preprocessor:
        // ms(..) = calculate boss timer value for amount of milliseconds
        x = x.replace(/ms\(([\d]+?)\)/g, (_, a) => Math.floor(Number(a) / 45))
        if (x.startsWith('on ')) {
            x = x.replace('on ', '')
            let evt = {}
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
                const sp = spawnpoints.find(y => y.name === x.trim())
                evt.spawn_point = sp.value
            } else if (x) {
                const params = x.split(',').map(y => y.split('=').map(z => z.trim()))
                params.forEach(p => {
                    evt[p[0]] = p[1]
                })
            }
            events.push(evt)
        } else if (x.startsWith('set ')) {
            x = x.replace('set ', '').split('=').map(y => y.trim())
            main[x[0]] = x[1]
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
        }
    })

const objToIni = obj => {
    return Object.keys(obj).map(key => key + '=' + obj[key]).join('\r\n') + '\r\n'
}

let str = `# Generated from ${fname}\r\n\r\n`

str += '[main]\r\n'

main.events = events.length

str += objToIni(main)

events.forEach((e, i) => {
    str += `\r\n[event_${i}]\r\n`
    str += objToIni(e)
})

spawnpoints.forEach((s, i) => {
    str += `\r\n[spawnpoint_${s.value}]\r\n`
    delete s.value
    delete s.name
    str += objToIni(s)
})

fs.writeFileSync(fname.replace('.boss', '.ini'), str)

