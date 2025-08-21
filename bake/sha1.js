// compile the sha1 module as separate step as it's a 3rd party component and we're not so interested
// in the compiler warnings
module.exports = v => v.sh(`cd src/sha1 && ${v.compiler} ${v.compiler_flags} -w -c sha1.c`)