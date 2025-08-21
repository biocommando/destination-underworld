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
    psykeily : Alien Spacecraft`
        
    names.split('\n').forEach(x => {
        const [origName, newName] = x.split(':').map(x => x.trim())
        fs.renameSync(targetDir + '/' + origName + '.mid', targetDir + '/' + newName + '.mid')
        fs.renameSync(targetDir + '/' + origName + '.mid_meta.ini', targetDir + '/' + newName + '.mid_meta.ini')
    })
}

module.exports = v => {
    const dest = 'DestinationUnderworldRelease/'

    fs.rmSync(dest, { recursive: true })

    fs.mkdirSync(`${dest}`)
    fs.mkdirSync(`${dest}dataloss`)
    fs.mkdirSync(`${dest}dataloss/midi-music`)
    fs.mkdirSync(`${dest}dataloss/core-pack`)
    fs.mkdirSync(`${dest}dataloss/editor`)
    fs.mkdirSync(`${dest}dataloss/editor/edit-res`)


    starCopy('dataloss/core-pack', /^mission.*$/, `${dest}dataloss/core-pack`)
    starCopy('dataloss/core-pack', /^(best_times|enemy-properties|arenas|help)\.dat$/, `${dest}dataloss/core-pack`)

    const isWindows = process.platform.includes('win')
    const mpAuthEx = isWindows ? '.\\mpauth.exe' : './mpauth'
    const mpAuthOut = v.sh(`${mpAuthEx} core-pack ./dataloss/core-pack/ no-debug-prints`).toString()
    fs.writeFileSync(`${dest}dataloss/core-pack/auth.dat`, mpAuthOut)

    starCopy('dataloss/editor', /\.(md|js|json|html)$/, `${dest}dataloss/editor`)
    starCopy('dataloss/editor/edit-res', /./, `${dest}dataloss/editor/edit-res`)

    starCopy('dataloss', /^(sprites|sounds|settings)\.dat$/, `${dest}dataloss`)

    starCopy('dataloss', /^white-rabbit\.TTF(-license\.txt)?$/, `${dest}dataloss`)

    starCopy('dataloss', /^sprites\.png$/, `${dest}dataloss`)
    starCopy('dataloss', /^(ending|hell)\.jpg$/, `${dest}dataloss`)

    starCopy('dataloss', /^(sel|warp|bt[1-2]|select_weapon|throw|healing|rune_of_protection|turret|blast|spawn|potion_(shield|stop|fast|boost|heal)|ex[1-6]|die[1-6]|wet_[1-3]|menusel|menuchg)\.ogg$/, `${dest}dataloss`)
    
    starCopy('dataloss/midi-music', /\.(mid|ini)$/, `${dest}dataloss/midi-music`)
    renameMidiFiles(`${dest}dataloss/midi-music`)

    starCopy('dataloss', /^wt_sample_slot_\d*\.wav$/, `${dest}dataloss`)

    starCopy(`${v.allegro_path}/bin`, /allegro_monolith-5.2/, dest)
    starCopy(`.`, /^DestinationUnderworld(\.exe)?$/, dest)
}