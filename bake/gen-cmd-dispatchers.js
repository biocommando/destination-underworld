module.exports = v => {
    v.fs.readdirSync('src/').filter(x => x.startsWith('dispatch_') && x.endsWith('.o'))
        .forEach(x => v.fs.rmSync(`src/${x}`))
    v.fs.readdirSync('src/command_file/generated').filter(x => x.startsWith('dispatch_'))
        .forEach(x => v.fs.rmSync(`src/command_file/generated/${x}`))
    v.sh('cd src/command_file/generated && node ../compiled_dispatcher.js ../du_commands.json')
    v.sh(`cd src && ${v.compiler} ${v.compiler_flags} ${v.allegro_cflags} -c ` +
        `command_file/generated/dispatch_*.c`)
    v.objFiles += ' ' + v.fs.readdirSync('src')
        .filter(x => x.startsWith('dispatch_') && x.endsWith('.o'))
        .map(x => `src/${x}`).join(' ')
}
