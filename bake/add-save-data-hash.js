module.exports = v => v.sh(`${v.compiler} ./src/add_save_data_hash.c ./src/record_file.c `
    + `./src/sha1/sha1.c ./src/sha1/du_dmac.c ./src/command_file/*.c ./src/command_file/generated/dispatch_record_file.c `
    + `./src/linked_list.c -Isrc/command_file -o add-save-data-hash`)