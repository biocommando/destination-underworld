function deserializeTileMap(fileAsString) {
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
            return { ifPresent: cb => cb(...tuple) };
        }
        return { ifPresent: () => { } };
    };

    const tileMap = {};
    nextTuple(1).ifPresent(version => Object.assign(tileMap, { version }));
    nextTuple(2).ifPresent((width, height) => Object.assign(tileMap, { width, height }));

    nextTuple(1).ifPresent(objCount => {
        const objects = [];
        while (objCount-- > 0) {
            nextTuple(4).ifPresent((id, x, y, room) => objects.push({ id, x, y, room }));
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

function serializeTileMap(tileMap, newline = '\n') {
    const lines = [];
    const zeroes = n => { let s = ''+n; for(n=n||1;n<1000;n*=10) s = '0' + s; return s; };
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
    objects.forEach(object => add([object.id, object.x, object.y, object.room]));
    add(tileMap.metadata.length);
    tileMap.metadata.forEach(add);
    add(JSON.stringify(tileMap.levelMetadata));
    return lines.join(newline);
}