module.exports = v => {
    v.sh(`${v.compiler} test/unittests.c -w -I test ${v.objFiles} ` +
    `src/synth/test/*.c src/test/*.c ${v.allegro_libs} ${v.libs} -o unittests.exe`)
    v.sh(`${v.compiler} test/test-du.c -o test-du.exe`)
    v.sh(`${v.compiler} -O2 src/synth/*.c -DSYNTH_COUNT_ACTIVE_VOICES ${v.libs} -o src/synth/synth_test_main.exe`)
}
