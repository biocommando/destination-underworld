module.exports = v => {
    let prefix = './'
    if (v.isWindows) {
        prefix = '.\\'
    }
    console.log('*** Run unit tests ***')
    let result = v.sh(`${prefix}unittests.exe`).toString()
    console.log(result)

    console.log('*** Run play tests ***')
    result = v.sh(`${prefix}test-du.exe test/tests.conf`).toString()
    result = result.match(/Test run done. Result: .*/)[0]
    console.log(result)
    if (result.includes('FAIL')) {
        console.log('Test log:')
        console.log(v.fs.readFileSync('test-results.txt').toString())
    }

    console.log('*** Run synth tests ***')
    v.sh(`cd src/synth && ${prefix}synth_test_main.exe ../../dataloss/midi-music/eetteritekno.mid`)
    const { createHash } = require('crypto');
    const sha256 = createHash('sha256').update(v.fs.readFileSync('dataloss/midi-music/eetteritekno.mid.wav')).digest('hex');
    const expected_sha256 = '058fd0b325f2697e104368cd5b9d0852789fa010c615424372927f82a231b56e'
    if (sha256 === expected_sha256)
        console.log('Synth test OK')
    else
        console.log('Synth test FAIL. SHA256 sum mismatch actual=', sha256, 'expected=', expected_sha256)
}
