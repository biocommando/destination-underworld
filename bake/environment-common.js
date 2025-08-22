module.exports = v => {
    v.allegro_path = 'allegro'
    v.compiler = 'gcc'
    v.isWindows = process.platform.includes('win')
    v.compiler_flags = '-O3 -std=c2x -Wall -Wextra -Wpedantic -Wno-unused-result -Wno-format-security'
    // Uncomment to remove warnings
    // v.compiler_flags += ' -w'
    v.auth_setup = ''
    v.mpAuthEx = v.isWindows ? '.\\mpauth.exe' : './mpauth'
    if (!v.isWindows) {
        const allegro_modules = [
            'font',
            'image',
            'audio',
            'ttf',
            'primitives',
            'acodec',
            'color'
        ].map(x => 'allegro_' + x + '-5').join(' ')
        v.allegro_cflags = v.sh(`pkg-config allegro-5 ${allegro_modules} --cflags`).toString().trim()
        v.allegro_libs = v.sh(`pkg-config allegro-5  ${allegro_modules} --libs`).toString().trim()
        v.libs = '-lm'
    } else {
        v.complier_flags += ' -static-libgcc'
        v.allegro_cflags = `-I../${v.allegro_path}/include`
        v.allegro_libs = `${v.allegro_path}/lib/liballegro_monolith.dll.a`
        v.libs = ''
    }
}
