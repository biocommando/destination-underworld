const fs = require('fs')

const lines = fs.readFileSync(process.argv[2]).toString().split('\n')
const outLines = []
let add = false
let idx = 0
let deltm, delfb
lines.forEach(x => {
    if (add) {
        const end = x.includes('SYNTH_DATA_END')
        if (!end) {
            let [key, val] = x.split('=')
            if (isNaN(val))
                return
            val = Number(val)
            if (key === 'deltm')
                deltm = val
            else if (key === 'delfb')
                delfb = val
            else if (key !== 'my_id' && key !== 'tempo')
                outLines.push(`${key}=${val}`)
        }
        else {
            outLines.push(x)
        }
        add = !end
    } else {
        add = x.includes('SYNTH_DATA_START')
        if (add) {
            idx++
        }
    }
})

outLines.unshift(`deltm=${deltm}`, `delfb=${delfb}`, `glob_gain_adj_db=0`)

fs.writeFileSync(process.argv[2].replace('.flp', '.mid_meta.ini'), outLines.join('\n') + '\n')