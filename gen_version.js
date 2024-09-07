fs = require('fs')

let v = fs.readFileSync('version_info.txt').toString()
v = v.replace(/[\r\n]/g, '')
v = `v. ${process.argv.pop()}+gitr${v}`

fs.writeFileSync('src/gen_version_info.h', `#pragma once

// file is generated

#define DU_VERSION "${v}"
`)