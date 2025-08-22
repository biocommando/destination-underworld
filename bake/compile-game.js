module.exports = v => {
    v.sh(`${v.mpAuthEx} core-pack ./dataloss/core-pack/ no-debug-prints > ./dataloss/core-pack/auth.dat`)
    v.sh(`cd src && ${v.compiler} ${v.compiler_flags} ${v.auth_setup} ` +
        `-c *.c sha1/du_dmac.c ${v.allegro_cflags}`)
    v.sh(`${v.compiler} ${v.compiler_flags} ${v.auth_setup} ${v.objFiles} ` +
        `src/main.o ${v.allegro_libs} ${v.libs} -o DestinationUnderworld.exe`)
}
