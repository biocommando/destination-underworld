module.exports = vars => {
    let v = vars.sh('git log --format=%h -n 1').toString()
    v = v.replace(/[\r\n]/g, '')
    v = `v. ${vars.version}+gitr${v}`

    vars.fs.writeFileSync('src/gen_version_info.h', `#pragma once

// file is generated

#define DU_VERSION "${v}"
`)
}