module.exports = v => {
    v.sh(`cd src/synth && ${v.compiler} ${v.compiler_flags} -I../command_file -Ofast -c *.c`)
}