module.exports = v => {
    v.sh(`${v.compiler} test/unittests.c -I test ${v.objFiles} ` +
    `src/synth/test/*.c src/test/*.c ${v.allegro_path}/lib/liballegro_monolith.dll.a -o unittests.exe`)
}