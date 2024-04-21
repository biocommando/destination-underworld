import sys

with open('version_info.txt') as f:
    v = f.read().strip()

v = f'v. {sys.argv[-1]}+gitr{v}'

with open('src/gen_version_info.h', 'w') as f:
    f.write('#pragma once\n')
    f.write('// file is generated\n')
    f.write(f'#define DU_VERSION "{v}"\n')
