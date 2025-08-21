module.exports = v => {
    v.sh(`cd src && ${v.compiler} ${v.compiler_flags} ${v.auth_setup} ` +
        `-c *.c sha1/du_dmac.c -I../${v.allegro_path}/include`)
    v.sh(`${v.compiler} ${v.compiler_flags} ${v.auth_setup} ${v.objFiles} ` +
        `src/main.o ${v.allegro_path}/lib/liballegro_monolith.dll.a -o DestinationUnderworld.exe`)
}