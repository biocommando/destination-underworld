const { serializeTileMap } = require('./tilemap-serialization')
const compileScript = require('./boss-to-ini-lib').bossToIni
const fs = require('fs')

const defaultParams = JSON.parse(fs.readFileSync('random-adventure-default-params.json'))
let customParams = {}

const configFile = process.argv.pop()
if (configFile.endsWith('.json')) customParams = JSON.parse(fs.readFileSync(configFile))

const params = { ...defaultParams, ...customParams }

const numMissions = params.numberOfMissions

const placeNames = [
    'Dungeon', 'Castle', 'Keep', 'Labyrinth', 'Cave', 'Cavern', 'Mansion', 'Manor',
    'Fortress', 'Fort', 'Woods', 'Forest', 'Halls', 'City', 'Ruins', 'Tower', 'Citadel',
    'Tunnels'
]

const ominiousWords = [
    'Doom', 'Evil', 'Destruction', 'Demons', 'Devil', 'Sin', 'Abominations', 'Monsters',
    'Damnation', 'Purgatory', 'Hell', 'Spirits', 'Hatred', 'Horrors', 'Terror', 'Enslavement',
    'Decapitation', 'Ghouls', 'Ghosts'
]
placeNames.sort(() => Math.random() - 0.5)
ominiousWords.sort(() => Math.random() - 0.5)

const packName = params.packName || `${placeNames.pop()}-of-${ominiousWords.pop()}--${new Date().getTime().toString(36)}`

function randomIntFromRange(range) {
    const [min, max] = range
    return Math.round(Math.random() * (max - min)) + min
}

const spawnerInfos = []

function createOneMission(mission) {

    console.log(`** Generating mission ${mission} **`)

    const ids = {
        wall: 120,
        floor: 46,
        lava: 122,
        wall2: 113,
        room: n => 1000 + n,
        exit: 60,
        enemy: e => e + 200,
        potion: p => p + 300,
        positionalTrigger: t => t + 600
    }

    let map = []
    let scripts = new Array(8).fill().map(() => [])

    function fillWithWalls(room) {
        for (let x = 0; x < 16; x++) {
            for (let y = 0; y < 12; y++) {
                map.push({ id: ids.wall, x, y, room })
            }
        }
    }

    function getTile(x, y, room) {
        return map.find(t => t.x === x && t.y === y && t.room === room)
    }

    function setTile(x, y, room, id) {
        const o = getTile(x, y, room)
        if (!o) console.log('not found:', x, y, room)
        o.id = id
    }

    function addBorders(room) {
        for (let x = 0; x < 16; x++) {
            for (let y = 0; y < 12; y++) {
                let tile
                if (x == 0 || x == 15 || y == 0 || y == 11) {
                    tile = ids.wall
                    if (x == 0 && y == 5) {
                        tile = ids.room(room == 1 ? room : room - 1)
                    }
                    if (x == 15 && y == 5 && room != 8) {
                        tile = ids.room(room + 1)
                    }
                    if (x == 15 && y > 3 && y < 9 && room == 8) {
                        tile = ids.exit
                    }
                }
                if (tile)
                    setTile(x, y, room, tile)
            }
        }
        const lavaThreshold = Math.random() * 0.2 + 0.7

        map.filter(x => x.id === ids.wall && x.room === room).forEach(x => {
            if (Math.random() > lavaThreshold) x.id = ids.lava
            else if (Math.random() > 0.9) x.id = ids.wall2
        })
    }

    function createEnemySpawnerScript(types, numEnemies, enemyProbs, room) {
        const script = []
        const enemyProbsStr = enemyProbs.join(', ')

        const possibleSpawnSpots = map.filter(t => t.id === ids.floor && t.room === room && t.x > 1)
        possibleSpawnSpots.sort(() => Math.random() - 0.5)
        types.forEach(type => {
            spawnerInfos.push({ room, mission, type })
            if (type === 'hydra') {
                const spawnSpot1 = possibleSpawnSpots.pop()
                const spawnSpot2 = possibleSpawnSpots.pop()
                script.push('// "Hydra" spawner')
                for (let i = 0; i < numEnemies; i++) {
                    script.push(`e.on(kill_count).is(${i + 1}).do(spawn).to('${spawnSpot1.x}, ${spawnSpot1.y}, ${enemyProbsStr}')`)
                    script.push(`e.on(kill_count).is(${i + 1}).do(spawn).to('${spawnSpot2.x}, ${spawnSpot2.y}, ${enemyProbsStr}')`)
                }
            } else if (type === 'timed') {
                const spawnSpot = possibleSpawnSpots.pop()
                script.push('// "Timed" spawner')
                script.push(`e.on(time_one_time).is(1).do(start_secondary_timer).to('time = 0')`)
                script.push(`e.on(time_one_time).is(ms(${2000 * numEnemies})).do(stop_secondary_timer).to('')`)
                script.push(`e.on(secondary_timer).is(ms(2000)).do(spawn).to('${spawnSpot.x}, ${spawnSpot.y}, ${enemyProbsStr}')`)
                script.push(`e.on(secondary_timer).is(ms(2000)).do(start_secondary_timer).to('time = 0')`)
            } else if (type === 'positional') {
                for (let y = 0; y < 12; y++) {
                    map.push({ x: 5, y, id: ids.positionalTrigger(0), room })
                }
                script.push('// "Positional" spawner')
                for (let i = 0; i < numEnemies; i++) {
                    const spawnSpot = possibleSpawnSpots.pop()
                    if (spawnSpot) {
                        script.push(`e.on(positional_trigger).is(0).do(spawn).to('${spawnSpot.x}, ${spawnSpot.y}, ${enemyProbsStr}')`)
                    }
                }
            } else {
                throw `Invalid spawner type ${type}. Allowed: hydra, timed, positional`
            }
        })
        scripts[room - 1] = script
    }

    let pausePotionSpawned = false
    for (let room = 1; room <= params.numberOfRooms; room++) {
        fillWithWalls(room)
        let x = 1, y = 4
        let enemies = 0, walkedCount = 0, chdirat = 2
        let enemyPowerScore = 0
        let dx = 0, dy = 1
        let bannedCoords = ['2,4', '2,5', '2,6']
        let totalHoleSizes = 0

        while (x !== 15 || y !== 5) {

            if (walkedCount >= chdirat) {
                let validDir = false, itCount = 0
                while (!validDir) {
                    const dir = Math.random()

                    dx = 0
                    dy = 0
                    if (dir < 0.25) dx = 1
                    else if (dir < 0.5) dx = -1
                    else if (dir < 0.75) dy = 1
                    else dy = -1

                    validDir = true
                    if (x + dx == 0)
                        validDir = false
                    if (x + dx == 15) {
                        if (y !== 5) validDir = false
                    }
                    if (y + dy == 0 || y + dy == 11)
                        validDir = false
                    const t = getTile(x + dx, y + dy, room)
                    if (!t || (t.id !== ids.wall && t.id !== ids.floor))
                        validDir = false
                    if (t && t.id !== ids.wall && itCount < 4)
                        validDir = false
                    itCount++
                }
                chdirat = Math.floor(Math.random() * params.keepWalkingDirectionMaxNumberOfTiles) + 1 + walkedCount
            }

            x += dx
            y += dy
            walkedCount++

            if (bannedCoords.includes(`${x},${y}`) || x < 0 || y < 0 || x > 15 || y > 11) {
                x -= dx
                y -= dy
                continue
            }

            if (x != 15 || y != 5)
                setTile(x, y, room, ids.floor)

            if (Math.random() > params.holeProbability && totalHoleSizes < params.holeTotalSizeLimit) {
                const r = Math.random()
                const holeSize = r * r * (params.maxHoleSize - 1) + 1
                totalHoleSizes += holeSize * holeSize * 0.5
                for (let xx = 0; xx < 15; xx++) {
                    for (let yy = 0; yy < 12; yy++) {
                        if (Math.sqrt((xx - x) * (xx - x) + (yy - y) * (yy - y)) < holeSize)
                            setTile(x, y, room, ids.floor)
                    }
                }
            }

            if ((x != 15 || y != 5) && x > 1
                && Math.random() > (1 - params.baseEnemyProbability) + Math.log(enemies) / (13 + room)) {
                const missionRoom = room + mission - 1
                let enemyTypeMax = params.baseEnemyTypeMax - 0.001
                if (missionRoom >= params.roomsThatIntroduceNewEnemyTypes[0]) enemyTypeMax += 1
                if (missionRoom >= params.roomsThatIntroduceNewEnemyTypes[1]) enemyTypeMax += 1
                if (missionRoom >= params.roomsThatIntroduceNewEnemyTypes[2]) enemyTypeMax += 1
                let enemyType = Math.floor(Math.random() * enemyTypeMax)
                map.push({ x, y, room, id: ids.enemy(enemyType) })
                enemies++
                enemyPowerScore += enemyType + 1
            }
        }

        const potionCount = enemyPowerScore >= params.potionSpawnEnemyPowerScoreThreshold ? 1 : 0

        for (let potions = 0; potions < potionCount; potions++) {
            let potionType = Math.floor(Math.random() * 5)
            if (potionType === 1 && params.limitPausePotionsTo1) {
                while (pausePotionSpawned && potionType === 1) {
                    potionType = Math.floor(Math.random() * 5)
                }
                pausePotionSpawned = true
            }

            let tile, x = 0, y = 0
            while (!tile || tile.id === ids.wall) {
                x = 1 + Math.floor(Math.random() * 14)
                y = 1 + Math.floor(Math.random() * 10)
                tile = getTile(x, y, room)
            }

            map.push({ x, y, room, id: ids.potion(potionType) })
        }

        addBorders(room)

        const floorTiles = map.filter(x => x.room === room && x.id === ids.floor).length
        if (floorTiles > params.tooOpenRoomFloorTileThreshold) {
            console.log('Too open map, regenerating')
            map = map.filter(x => x.room !== room)
            room--
            continue
        }

        console.log(`Room ${room}, spawned enemies: ${enemies}, potions: ${potionCount}, walkedCount: ${walkedCount}`)
    }

    map.push({ x: 1, y: 5, id: ids.potion(5), room: 1, condition: 'COND_POTION_ONLY' })

    const roomsWithSpawners = []
    let numSpawners = randomIntFromRange(params.spawners.numSpawnersPerMapRange)
    console.log(`Creating ${numSpawners} spawners`)
    for (let i = 0; i < numSpawners; i++) {
        let room
        while (!room || roomsWithSpawners.includes(room)) {
            room = randomIntFromRange([1, 8])
        }
        roomsWithSpawners.push(room)
        const type = params.spawners.allowedSpawnerTypes[randomIntFromRange([0, params.spawners.allowedSpawnerTypes.length - 1])]
        const numEnemies = randomIntFromRange(params.spawners.numEnemiesRange)
        createEnemySpawnerScript([type], numEnemies, params.spawners.enemyProbabilities, room)
    }

    let colors
    do {
        colors = [Math.pow(Math.random(), 2), Math.pow(Math.random(), 2), Math.pow(Math.random(), 1)]
    } while (!colors.some(x => x > 0.5))
    colors.sort(() => Math.random() - 0.5)

    const metadata = [
        `name = "${packName.split('--')[0].replace(/-/g, ' ')} ${mission} / ${numMissions}"`,
        `wall_color = "${colors.join(' ')}"`,
        'condition: COND_POTION_ONLY: game_modifiers = 200',
        'mute_bosstalk = "1"',
    ]

    const compiledScripts = scripts.map(s => compileScript(s.join('\n'), 'new'))

    const serialized = serializeTileMap({
        objects: map,
        scripts,
        compiledScripts,
        metadata
    }).replace(/\n/, '\r\n')

    fs.writeFileSync(packName + '/mission' + mission, serialized)
}

fs.mkdirSync(packName)

for (let m = 0; m < numMissions; m++) {
    createOneMission(m + 1)
}

let enemyProperties = `# Adept
type="0" turret="0"  rate="25" health="2"  gold="0"  fast="0"  hurts-monsters="0"  potion-for-potion-only="5"
# Magician
type="1" turret="0"  rate="20" health="3"  gold="0"  fast="0"  hurts-monsters="0"  potion-for-potion-only="2"
# Imp
type="2" turret="0"  rate="15" health="5"  gold="1"  fast="1"  hurts-monsters="0"  potion-for-potion-only="4"
# Alien
type="3" turret="0"  rate="10" health="6"  gold="1"  fast="1"  hurts-monsters="0"  potion-for-potion-only="3"
# Alien turret
type="4" turret="1"  rate="5"  health="8"  gold="1"  fast="0"  hurts-monsters="1"  potion-for-potion-only="3"
`

let enemyHelp = ''

if (!params.randomEnemyProperties.useDefault) {
    enemyHelp = `(empty line)
Custom enemy profiles
color_ref: "1"
sprite: "17" "10" "55" "0" "0"
sprite: "17" "10" "87" "0" "1"
sprite: "17" "10" "119" "0" "2"
sprite: "17" "10" "151" "0" "3"
sprite: "17" "10" "183" "0" "4"
margin: "40"
(empty line)
`
    enemyProperties = ''
    for (let e = 0; e < 5; e++) {
        let powerScore = 1e9
        let turret, rate, health, gold, fast, hurtsMonsters, potionForPotionOnly
        const potionSelection = [2, 3, 4, 5]
        let offset = params.randomEnemyProperties.allowedPowerScoreOffset
        let it = 0
        while (Math.abs(powerScore - params.randomEnemyProperties.targetPowerScores[e]) > offset) {
            turret = e === 4 ? 1 : 0
            rate = randomIntFromRange(params.randomEnemyProperties.rateRange)
            health = randomIntFromRange(params.randomEnemyProperties.healthRange)
            fast = Math.random() > params.randomEnemyProperties.fastProbability ? 1 : 0
            if (turret) fast = 0
            hurtsMonsters = Math.random() > params.randomEnemyProperties.hurtsMonstersProbability ? 1 : 0
            potionSelection.sort(() => Math.random() - 0.5)
            potionForPotionOnly = potionSelection[0]

            powerScore = (30 - rate) / 5 + health / 2 + fast * 2 - turret - hurtsMonsters

            gold = powerScore > params.randomEnemyProperties.goldAtPowerScore ? 1 : 0
            if (++it === 1000) {
                offset *= 2
                it = 0
            }
        }

        enemyProperties += [e, turret, rate, health, gold, fast, hurtsMonsters, potionForPotionOnly].map(x => '"' + x + '"').join('') + '\n'
        enemyHelp += `Health\\. ${health}, Fire rate\\. ${Math.round(400 / rate * (fast + 1)) / 10} / sec, ${turret ? 'Stationary' : (fast ? 'Moves fast' : 'Moves slow')}${hurtsMonsters ? ', Friendly fire' : ''}${gold ? ', Possesses a soul' : ''}
(empty line)
`
        console.log(`Enemy ${e} created with power score ${powerScore}`)
    }
    enemyHelp += 'margin: "5"'
}

fs.writeFileSync(packName + '/arenas.dat', '')
fs.writeFileSync(packName + '/enemy-properties.dat', enemyProperties)
const gameTuning = fs.readFileSync('../core-pack/game-tuning.dat')
fs.writeFileSync(packName + '/game-tuning.dat', gameTuning)

spawnerInfos.sort((a, b) => a.mission * 10 + a.room - b.mission * 10 - b.room)

function getSpawnerInfoTable() {
    let rows = ''
    for (let mission = 1; mission <= numMissions; mission++) {
        const infos = spawnerInfos.filter(x => x.mission === mission)
        const rooms = [1, 2, 3, 4, 5, 6, 7, 8].map(room => {
            const info = infos.find(i => i.room === room)
            if (info) {
                return { hydra: ' HYDRA  ', positional: ' AMBUSH ', timed: ' REINF. ' }[info.type]
            } else {
                return '        '
            }
        }).join('|')
        const paddedMission = (mission + ' ').substring(0, 2)
        rows += `  ${paddedMission}  |${rooms}
`
    }
    return rows
}

fs.writeFileSync(packName + '/help.dat', `color: "128" "128" "64" "1"
A random-generated dungeon pack for Destination Underworld.
color: "192" "64" "32" "2"
${enemyHelp}
(empty line)
color_ref: "2"
Map trap info
color: "64" "64" "64" "3"
HYDRA         = When an enemy is killed, two are spawned at a random location
AMBUSH        = When the player gets near the middle of the level,
                enemies spawn at random locations
REINFORCEMENT = New enemies are spawned in the same location periodically
(empty line)
color_ref: "1"
LEVEL | ROOM 1 |    2   |    3   |    4   |    5   |    6   |    7   |    8
color_ref "3"
${getSpawnerInfoTable()}
color: "255" "255" "255" "4"
(empty line)
((press space to continue))
doc_end
`)

fs.writeFileSync(packName + '/readme.txt', `Running the game with this custom mission pack:
1. Copy the directory ${packName} under data directory
2. Run the ${packName}.bat file`)

fs.writeFileSync(packName + '/' + packName + '.bat', `
IF EXIST mission1 (
    cd ..
    cd ..
)
DestinationUnderworld.exe --general--mission-pack=${packName} --default-game-mode=${params.defaultGameMode}`)

fs.writeFileSync(packName + '/mission-counts.dat', 'initial_count: "' + numMissions + '"')

console.log(`Created files for pack named ${packName}`)