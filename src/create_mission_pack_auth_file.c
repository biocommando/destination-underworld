#include "sha1/du_dmac.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("Arguments required: mission pack name and path (with terminating slash, use slash instead of backslash)\n");
        return 1;
    }
    int debug_prints = 1;
    if (argc > 3 && !strcmp(argv[3], "no-debug-prints"))
        debug_prints = 0;

    char node_cmd[512];
    sprintf(node_cmd, "node -e \""
        "s=fs.readdirSync('%s').join('\\n');fs.writeFileSync('files.txt',s)"
        "\"", argv[2]);
    if (debug_prints)
        printf("Executing command:\n%s\n", node_cmd);
    system(node_cmd);
    FILE *f = fopen("files.txt", "r");
    if (debug_prints)
        printf("File contents:\n");
    while (!feof(f))
    {
        char path[256], fullpath[256];
        fgets(path, 256, f);
        if (feof(f))
            break;
        // Whitelisting
        if (!strstr(path, "mission") && !strstr(path, "enemy-properties.dat") && !strstr(path, "game-tuning.dat"))
            continue;
        if (path[strlen(path) - 1] == '\n')
            path[strlen(path) - 1] = 0;
        sprintf(fullpath, "%s%s", argv[2], path);
        char hash[DMAC_SHA1_HASH_SIZE];
        dmac_sha1_set_ctx(0);
        dmac_sha1_calculate_hash_f(hash, fullpath);
        char hex[100];
        convert_sha1_hash_to_hex(hex, hash);
        printf("\"./dataloss/%s/%s\" \"%s\"\n", argv[1], path, hex);
    }
    fclose(f);
    return 0;
}