const fs = require('fs')

function starCopy(dir, regex, destination) {
    let list = fs.readdirSync(dir)
    list = list.filter(x => x.match(regex))
    list.forEach(file => fs.copyFileSync(`${dir}/${file}`, `${destination}/${file}`))
}

function renameMidiFiles(targetDir) {
    let names = `3xoscs : Demon Slayer Deluxe
    abyss : Exploring the Abyss
    Hi Tim! : Hi Tim!
    hspb_synth_acid : Iku-Turso's Acid Trip
    korg seq 90s anime : Theme from 'Atsushi the Rescue Dog'
    life_after_death : Life After Death
    miditracktest : Space Exploration Gone Wrong
    sikanaamari : We're All Pigs Behind Our Masks
    soft_values : Soft Values, Hardcore Attitude
    sytrus presets : Evening Tea at the Local Space Port Lounge
    trance : Elevate
    weird : Trip to the Demon Zoo
    with new wavetables : Nuclear Waste Hangover
    eetteritekno : Relieve Us
    it tastes funky : Rancid Demon Food
    du ost : Ode to Underworld
    du ost 3 : 8-bit Apocalypse
    psykeily : Alien Spacecraft
    2025-11-dutheme : Weird Horror
    2025-11-chill-melody : Wild Walrus on a Vacation
    2025-11-dnb : Devil in the Details
    prob2 : Lord of Lies
    Project_1 : Battle Bot Disassembly`

    names.split('\n').forEach(x => {
        const [origName, newName] = x.split(':').map(x => x.trim())
        fs.renameSync(targetDir + '/' + origName + '.mid', targetDir + '/' + newName + '.mid')
        fs.renameSync(targetDir + '/' + origName + '.mid_meta.ini', targetDir + '/' + newName + '.mid_meta.ini')
    })
}

module.exports = v => {
    const dest = 'DestinationUnderworldRelease/'

    if (fs.existsSync(dest)) {
        fs.rmSync(dest, { recursive: true })
    }

    fs.mkdirSync(`${dest}`)
    fs.mkdirSync(`${dest}data`)
    fs.mkdirSync(`${dest}data/midi-music`)
    fs.mkdirSync(`${dest}data/editor`)
    fs.mkdirSync(`${dest}data/editor/edit-res`)

    function copyMissionPackEssentials(packName) {
        fs.mkdirSync(`${dest}data/${packName}`)
        starCopy('data/' + packName, /^mission\d*$/, `${dest}data/${packName}`)
        starCopy('data/' + packName, /^mission\d*-mode-\d*$/, `${dest}data/${packName}`)
        starCopy('data/' + packName, /^(enemy-properties|arenas|help|game-tuning|sprites|sounds|mission-counts|pack-name)\.dat$/, `${dest}data/${packName}`)
    }

    copyMissionPackEssentials('core-pack')

    fs.copyFileSync('data/core-pack/best_times.dat.template', `${dest}data/core-pack/best_times.dat`)

    const mpAuthOut = v.sh(`${v.mpAuthEx} core-pack ./data/core-pack/ no-debug-prints`).toString()
    fs.writeFileSync(`${dest}data/core-pack/auth.dat`, mpAuthOut)

    starCopy('data/editor', /\.(md|js|json|html)$/, `${dest}data/editor`)
    starCopy('data/editor/edit-res', /./, `${dest}data/editor/edit-res`)

    starCopy('data', /^(sprites|sounds|settings)\.dat$/, `${dest}data`)

    starCopy('data', /^white-rabbit\.TTF(-license\.txt)?$/, `${dest}data`)

    starCopy('data', /^sprites\.png$/, `${dest}data`)
    starCopy('data', /^(ending|hell)\.jpg$/, `${dest}data`)

    starCopy('data', /^(sel|warp|bt[1-2]|select_weapon|throw|healing|rune_of_protection|turret|blast|spawn|potion_(shield|stop|fast|boost|heal)|ex[1-6]|die[1-6]|wet_[1-3]|menusel|menuchg)\.ogg$/, `${dest}data`)

    copyMissionPackEssentials('robot-uprising')
    starCopy('data/robot-uprising', /^(disassemble(|2)|parts_scatter)\.ogg$/, `${dest}data/robot-uprising`)
    starCopy('data/robot-uprising', /^sprites\.png$/, `${dest}data/robot-uprising`)

    copyMissionPackEssentials('classic')

    starCopy('data/midi-music', /\.(mid|ini)$/, `${dest}data/midi-music`)
    renameMidiFiles(`${dest}data/midi-music`)

    starCopy('data', /^wt_sample_slot_\d*\.wav$/, `${dest}data`)

    if (v.isWindows) {
        starCopy(`${v.allegro_path}/bin`, /allegro_monolith-5.2/, dest)
        /*fs.writeFileSync(`${dest}RobotUprising.bat`, 'DestinationUnderworld.exe --general--mission-pack=robot-uprising')
        fs.writeFileSync(`${dest}Classic.bat`, 'DestinationUnderworld.exe --general--mission-pack=classic')*/
    }
    starCopy(`.`, /^DestinationUnderworld(\.exe)?$/, dest)
}
