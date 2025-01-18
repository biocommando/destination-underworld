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
with new wavetables : Nuclear Waste Hangover`

const fs = require('fs')

const targetDir = process.argv.pop()

names.split('\n').forEach(x => {
    const [origName, newName] = x.split(':').map(x => x.trim())
    fs.renameSync(targetDir + '/' + origName + '.mid', targetDir + '/' + newName + '.mid')
    fs.renameSync(targetDir + '/' + origName + '.mid_meta.ini', targetDir + '/' + newName + '.mid_meta.ini')
})