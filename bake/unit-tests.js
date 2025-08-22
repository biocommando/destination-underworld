module.exports = v => {
    v.sh(`${v.compiler} test/unittests.c -w -I test ${v.objFiles} ` +
    `src/synth/test/*.c src/test/*.c ${v.allegro_libs} ${v.libs} -o unittests.exe`)
}
