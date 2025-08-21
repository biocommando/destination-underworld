module.exports = v => {
    v.allegro_path = 'allegro'
    v.compiler = 'gcc'
    v.compiler_flags = '-static-libgcc -O3 -Wall -Wextra -Wpedantic -std=c2x'
    v.auth_setup = ''
}