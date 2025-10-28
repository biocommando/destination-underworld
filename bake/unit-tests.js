module.exports = v => {
    v.sh(`${v.compiler} test/unittests.c -w -I test ${v.objFiles} ` +
    `src/synth/test/*.c src/test/*.c ${v.allegro_libs} ${v.libs} -o unittests.exe`)
    v.sh(`${v.compiler} test/test-du.c -o test-du.exe`)
    const sObjFiles = `src/synth/*.o src/linked_list.o src/command_file.o src/dispatch_synth_params.o src/strescape.o`
    v.sh(`${v.compiler} -Isrc/command_file ${sObjFiles} ${v.libs} -o src/synth/synth_test_main.exe`)
}
