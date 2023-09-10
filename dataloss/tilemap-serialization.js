function deserializeTileMap(fileAsString) {
    const lines = fileAsString.split(/\r?\n/);
    if (lines[0] !== 'X(duedit_compatible)') {
        return deserializeTileMapLegacy(fileAsString)
    }
    lines.shift()
    const data = { objects: [], metadata: [], height: 12, width: 16 }
    let section = '', conditionName
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
            } else if (section === 'condition') {
                section = ''
            }
        }
        else {
            if (section === 'tiles') {
                const [id, x, y, room] = line.split(' ').map(x => Number(x))
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
                data.objects.push({ id, x, y, room, condition: conditionName })
            }
            if (section === 'metadata' && line.startsWith('*')) {
                const s = line.substring(2).split('"')
                data.metadata.push(`${s[0]} = "${s[1]}"`)
            }
        }
    })
    return data
}

function serializeTileMap(data, newline = '\n') {
    const lines = []
    function serializeList(list) {
        list.forEach(obj => {
            lines.push(`${obj.id} ${obj.x} ${obj.y} ${obj.room}`)
        })
    }
    const conditions = data.metadata.filter(x => x.startsWith('condition: '))
        .map(line => {
            const s = line.split(':').map(x => x.trim())
            if (!s[1].startsWith('COND_')) alert('Condition names should start with string "COND_"')
            return { name: s[1], condition: s[2] }
        })
    lines.push('X(duedit_compatible)')
    lines.push('*@tiles')
    serializeList(data.objects.filter(x => !x.condition))
    conditions.forEach(item => {
        lines.push('#conditions')
        lines.push(`*?${item.condition} ${item.name}`)
        lines.push(`*>+SKIP_${item.name}`)
        lines.push(`*@${item.name}`)
        serializeList(data.objects.filter(x => x.condition === item.name))
        lines.push(`*@+SKIP_${item.name}`)
    })
    lines.push('*?level_read = 1 +end')
    lines.push('*@metadata')
    data.metadata
        .filter(x => !x.startsWith('condition: ') && x.indexOf('=') > 0)
        .forEach(meta => {
            const eqidx = meta.indexOf('=')
            const variable = meta.substring(0, eqidx).trim()
            const value = meta.substring(eqidx + 1).trim()
            lines.push(`*=${variable}${value}`)
        })
        lines.push('*@+end')
        lines.push('')
    return lines.join(newline)
}

function deserializeTileMapLegacy(fileAsString) {
    const lines = fileAsString.split(/\r?\n/);
    let count = 0;
    const nextLine = () => lines[count++];
    const nextTuple = (length) => {
        const line = nextLine();
        const tuple = line.split(' ').map(Number);
        if (length === undefined) {
            return tuple;
        }
        if (length <= tuple.length &&
            !tuple.some((v, i) => i < length && isNaN(v))) {
            return { ifPresent: cb => cb(...tuple, line) };
        }
        return { ifPresent: () => { } };
    };

    const tileMap = {};
    nextTuple(1).ifPresent(version => Object.assign(tileMap, { version }));
    nextTuple(2).ifPresent((width, height) => Object.assign(tileMap, { width, height }));

    nextTuple(1).ifPresent(objCount => {
        const objects = [];
        while (objCount-- > 0) {
            nextTuple(4).ifPresent((id, x, y, room, n, line) => {
                const o = { id, x, y, room }
                if (line)
                    o.condition = line.split(' ').pop()
                objects.push(o)
            });
        }
        Object.assign(tileMap, { objects });
    });

    nextTuple(1).ifPresent(metadataCount => {
        const metadata = [];
        while (metadataCount-- > 0) {
            metadata.push(nextLine());
        }
        Object.assign(tileMap, { metadata });
    });

    return Object.assign(tileMap, { levelMetadata: JSON.parse(nextLine()) });
}

function serializeTileMapLegacy(tileMap, newline = '\n') {
    const lines = [];
    const zeroes = n => { let s = '' + n; for (n = n || 1; n < 1000; n *= 10) s = '0' + s; return s; };
    const zpad = n => typeof n === 'number' ? zeroes(n) : n;
    const add = o => { if (typeof o !== 'object') lines.push(zpad(o)); else lines.push(o.map(zpad).join(' ')); }
    add(tileMap.version);
    add([tileMap.width, tileMap.height]);
    const objects = tileMap.objects.sort((a, b) => {
        const idxA = tileMap.width * a.y + a.x;
        const idxB = tileMap.width * b.y + b.x;
        if (a.room != b.room) return a.room - b.room;
        return idxA - idxB === 0 ? a.id - b.id : idxA - idxB;
    });
    add(objects.length);
    objects.forEach(object => {
        const a = [object.id, object.x, object.y, object.room]
        if (object.condition)
            a.push(object.condition)
        add(a)
    });
    add(tileMap.metadata.length);
    tileMap.metadata.forEach(add);
    add(JSON.stringify(tileMap.levelMetadata));
    return lines.join(newline);
}