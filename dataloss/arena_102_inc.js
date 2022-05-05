let all_spawnpoints = {}

function create_spawnpoints(prefix) {
    let template = 'def spawnpoint N = X, Y, 50, 30, 10, 10, 0'
    all_spawnpoints[prefix] = []
    if (prefix === 'easy') {
        template = 'def spawnpoint N = X, Y, 40, 20, 5, 5, 0'
    }
    for (let x = 5; x <= 9; x++)
    {
        for (let y = 3; y <= 7; y++)
        {
            if (x === 5 || x === 9 || y === 3 || y === 7)
            {
                all_spawnpoints[prefix].push(template
                    .replace('N', prefix + '#' + all_spawnpoints[prefix].length)
                    .replace('X', x)
                    .replace('Y', y))
            }
        }
    }
}

module.exports = {
    spawnpoint: (n, prefix = '') => {
        if (!all_spawnpoints[prefix])
            create_spawnpoints(prefix)
        return all_spawnpoints[prefix][n]
    }
}