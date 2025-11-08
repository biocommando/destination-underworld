function deserializeTileMap(fileAsString) {
    if (fileAsString[0] === 'X')
        return deserializeTileMapOld(fileAsString)
    return deserializeTileMapNew(fileAsString)
}

function deserializeTileMapOld(fileAsString) {
    const lines = fileAsString.split(/\r?\n/);
    if (lines[0] !== 'X(duedit_compatible)') {
        throw 'Incompatible level file'
    }
    lines.shift()
    const data = {
        objects: [], metadata: [], height: 12, width: 16,
        scripts: [[], [], [], [], [], [], [], []],
        compiledScripts: ['', '', '', '', '', '', '', '']
    }
    let section = '', conditionName, script, compiledScript
    lines.forEach(line => {
        if (line.startsWith('*@') || line.startsWith('#')) {
            if (line === '*@tiles') {
                section = 'tiles'
            } else if (line === '#conditions') {
                section = 'conditions'
            } else if (line.startsWith('*@COND_')) {
                section = 'condition'
                conditionName = line.split('*@')[1]
            } else if (line.startsWith('*@metadata')) {
                section = 'metadata'
                data.metadata.push('')
            } else if (line.startsWith('#script ')) {
                const room = Number(line.split(' ').pop())
                script = data.scripts[room - 1]
                section = 'script'
            } else if (section === 'script') {
                if (line === '#end_script') {
                    section = ''
                } else {
                    script.push(line.substring(2))
                }
            } else if (section === 'condition') {
                section = ''
            } else if (line.startsWith('$')) {
                section = 'compiled_script'
                const room = Number(line.split('$').pop())
                compiledScript = data.compiledScripts[room - 1]
            }
        }
        else {
            if (section === 'tiles') {
                const [id, x, y, room] = line.split(' ').map(x => Number(x))
                if (!isNaN(id))
                    data.objects.push({ id, x, y, room })
            }
            if (section === 'conditions' && line[1] === '?') {
                const s = line.split(' ')
                const name = s.pop()
                s[0] = s[0].substring(2)
                data.metadata.push(`condition: ${name}: ${s.join(' ')}`)
            }
            if (section === 'condition') {
                const [id, x, y, room] = line.split(' ').map(x => Number(x))
                if (!isNaN(id))
                    data.objects.push({ id, x, y, room, condition: conditionName })
            }
            if (section === 'metadata' && line.startsWith('*')) {
                const s = line.substring(2).split('"')
                data.metadata.push(`${s[0]} = "${s[1]}"`)
            }
            if (section === 'compiled_script') {

            }
        }
    })
    return data
}

const parseCmdLine = line => {
	let [command, params] = line.split(':')
        if (params)
		params = params.match(/"[^"]*("|$)/g).map(x => x.replace(/"/g, ''))
	else params = []
        return {command, params}
}

function deserializeTileMapNew(fileAsString) {
    const lines = fileAsString.split(/\r?\n/);
    const hdr = parseCmdLine(lines[0])
    if (hdr.command !== 'header' || hdr.params[0] !== '3' || hdr.params[1] !== 'duedit_compatible') {
        throw 'Incompatible level file'
    }
    lines.shift()
    const data = {
        objects: [], metadata: [], height: 12, width: 16,
        scripts: [[], [], [], [], [], [], [], []],
        compiledScripts: ['', '', '', '', '', '', '', '']
    }
    let section = '', conditionName, script, compiledScript
    lines.forEach(line => {
        if (line.startsWith(':') || line.startsWith('#')) {
            if (line === ':tiles') {
                section = 'tiles'
            } else if (line === '#conditions') {
                section = 'conditions'
            } else if (line.startsWith(':COND_')) {
                section = 'condition'
                conditionName = line.split(':')[1]
            } else if (line.startsWith(':metadata')) {
                section = 'metadata'
                data.metadata.push('')
            } else if (line.startsWith('#script ')) {
                const room = Number(line.split(' ').pop())
                script = data.scripts[room - 1]
                section = 'script'
            } else if (section === 'script') {
                if (line === '#end_script') {
                    section = ''
                } else {
                    script.push(line.substring(2))
                }
            } else if (section === 'condition') {
                section = ''
            } else if (line.startsWith('script_start:')) {
                section = 'compiled_script'
                const room = Number(parseCmdLine(line).params[0])
                compiledScript = data.compiledScripts[room - 1]
            }
        }
        else {
            if (section === 'tiles') {
                const [id, x, y, room] = parseCmdLine(line).params.map(x => Number(x))
                if (!isNaN(id))
                    data.objects.push({ id, x, y, room })
            }
            if (section === 'conditions' && line.startsWith('condition:')) {
                const s = parseCmdLine(line).params
                const name = s.pop()
                //s[0] = s[0].substring(2)
                data.metadata.push(`condition: ${name}: ${s.join(' ')}`)
            }
            if (section === 'condition') {
                const [id, x, y, room] = parseCmdLine(line).params.map(x => Number(x))
                if (!isNaN(id))
                    data.objects.push({ id, x, y, room, condition: conditionName })
            }
            if (section === 'metadata' && line.startsWith('set_var:')) {
                const s = parseCmdLine(line).params
                data.metadata.push(`${s[0]} = "${s[1]}"`)
            }
            if (section === 'compiled_script') {

            }
        }
    })
    return data
}

function serializeTileMap(data, newline = '\n') {
    const lines = []
    data.objects.sort((a, b) => {
        if (a.room !== b.room)
            return a.room - b.room
        if (a.y !== b.y)
            return a.y - b.y
        if (a.x !== b.x)
            return a.x - b.x
        if (a.id !== b.id)
            return a.id - b.id
        return 0
    })
    function serializeList(list) {
        list.forEach(obj => {
            lines.push(`object: "${obj.id}" "${obj.x}" "${obj.y}" "${obj.room}"`)
        })
    }
    const conditions = data.metadata.filter(x => x.startsWith('condition: '))
        .map(line => {
            const s = line.split(':').map(x => x.trim())
            if (!s[1].startsWith('COND_')) alert('Condition names should start with string "COND_"')
            return { name: s[1], condition: s[2] }
        })
    lines.push('header: "3" "duedit_compatible"')
    lines.push(':tiles')
    serializeList(data.objects.filter(x => !x.condition))
    conditions.forEach(item => {
        lines.push('#conditions')
        lines.push(`condition: "${item.condition}" "${item.name}"`)
        lines.push(`goto: "+SKIP_${item.name}"`)
        lines.push(`:${item.name}`)
        serializeList(data.objects.filter(x => x.condition === item.name))
        lines.push(`:+SKIP_${item.name}`)
    })
    lines.push(':metadata')
    data.metadata
        .filter(x => !x.startsWith('condition: ') && x.indexOf('=') > 0)
        .forEach(meta => {
            const eqidx = meta.indexOf('=')
            const variable = meta.substring(0, eqidx).trim()
            const value = meta.substring(eqidx + 1).trim()
            lines.push(`set_var: "${variable}" ${value}`)
        })
    data.scripts.forEach((script, i) => {
        if (script.length > 0) {
            lines.push(`#script ${i + 1}`)
            lines.push(...script.map(x => `# ${x}`))
            lines.push('#end_script')
        }
    })
    data.compiledScripts.forEach((script, i) => {
        if (script.length > 0) {
            lines.push(`script_start: "${i + 1}"`)
            lines.push(...script.split('\n').map(x => `script_line: "${x}"`))
        }
    })
    lines.push('')
    return lines.join(newline)
}

try {
    module.exports = {
        serializeTileMap, deserializeTileMap
    }
} catch (e) {
    // ok
}