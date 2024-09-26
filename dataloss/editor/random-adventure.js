const {serializeTileMap} = require('./tilemap-serialization')
const fs = require('fs')

const numMissions = 4
const packName = 'random-dungeon-' + new Date().getTime().toString(36)

function createOneMission(mission) {

const ids = {
    wall: 120,
    floor: 46,
    lava: 122,
    wall2: 113,
    room: n => 1000 + n,
    exit: 60,
    enemy: e => e + 200,
    potion: p => p + 300
}

const map = []

// Fill with walls

for (let room = 1; room <= 8; room++) {
    for (let x = 0; x < 16; x++) {
        for (let y = 0; y < 12; y++) {
            map.push({id: ids.wall, x, y, room})
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


let pausePotionSpawned = false
for (let room = 1; room <= 8; room++) {
    let x = 1, y = 4
    let enemies = 0, walkedCount = 0, chdirat = 2
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
            chdirat = Math.floor(Math.random() * 4) + 1 + walkedCount
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

        if (Math.random() > 0.95 && totalHoleSizes < 50) {
            const r = Math.random()
            const holeSize = r * r * 5 + 1
            totalHoleSizes += holeSize * holeSize * 0.5
            for (let xx = 0; xx < 15; xx++) {
                for (let yy = 0; yy < 12; yy++) {
                    if (Math.sqrt((xx - x) * (xx - x) + (yy - y) * (yy - y)) < holeSize)
                        setTile(x, y, room, ids.floor)
                }
            }
        }

        if (x > 1 && Math.random() > 0.85 + Math.log(enemies) / (13 + room)) {
            let enemyTypeMax = 1.9999
            if (room > 2) enemyTypeMax = 2.999
            if (room > 4) enemyTypeMax = 3.999
            if (room > 6) enemyTypeMax = 4.999
            let enemyType = Math.floor(Math.random() * enemyTypeMax)
            map.push({x, y, room, id: ids.enemy(enemyType)})
            enemies++
        }
    }

    for (let potions = 0; potions < Math.floor(enemies / 7); potions++) {
        let potionType = Math.floor(Math.random() * 5)
        if (potionType === 1) {
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

        map.push({x, y, room, id: ids.potion(potionType)})
    }

    console.log(`Room ${room}, spawned enemies: ${enemies}, walkedCount: ${walkedCount}`)
}

// Borders
for (let room = 1; room <= 8; room++) {
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

const colors = [Math.pow(Math.random(), 2), Math.pow(Math.random(), 2), Math.pow(Math.random(), 1)]
colors.sort(() => Math.random() - 0.5)

const metadata = [`name = "Dungeon ${mission} / ${numMissions}"`, `wall_color = "${colors.join(' ')}"`]

if (mission === numMissions) {
    metadata.push('no_more_levels = "1"')
}

const serialized = serializeTileMap({
    objects: map,
    scripts: [],
    compiledScripts: [],
    metadata
}).replace(/\n/, '\r\n')

fs.writeFileSync(packName + '/mission' + mission, serialized)
}

fs.mkdirSync(packName)

for (let m = 0; m < numMissions; m++) {
    createOneMission(m + 1)
}

fs.writeFileSync(packName + '/arenas.dat', 'number_of_arenas 0')
fs.writeFileSync(packName + '/enemy-properties.dat', `# Adept
type-0 turret=0 rate=25 health=2 gold=0 fast=0 hurts-monsters=0 potion-for-potion-only=5
# Magician
type-1 turret=0 rate=20 health=3 gold=0 fast=0 hurts-monsters=0 potion-for-potion-only=2
# Imp
type-2 turret=0 rate=15 health=5 gold=1 fast=1 hurts-monsters=0 potion-for-potion-only=4
# Alien
type-3 turret=0 rate=10 health=6 gold=1 fast=1 hurts-monsters=0 potion-for-potion-only=3
# Alien turret
type-4 turret=1 rate=5 health=8 gold=1 fast=0 hurts-monsters=1 potion-for-potion-only=3`)
fs.writeFileSync(packName + '/help.dat', `A random-generated dungeon pack for Destination Underworld.


((press space to continue))
#doc-end
`)

fs.writeFileSync(packName + '/readme.txt', `Running the game with this custom mission pack:
1. Copy the directory ${packName} under dataloss directory
2. Copy the ${packName}.bat under root directory
3. Run the bat file`)

fs.writeFileSync(packName + '/' + packName + '.bat', `DestinationUnderworld.exe --general--mission-pack=${packName} --${packName}--mission-count=4`)