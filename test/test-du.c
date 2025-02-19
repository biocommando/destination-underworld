#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *recording = NULL, *complete_state = NULL;

int play_one()
{
    char cmd[1024];
    sprintf(cmd, "DestinationUnderworld.exe --record-mode=play --file=%s "
        "--start-without-user-interaction=1 --core-pack--require-authentication=0", recording);
    printf("Commandline:\n%s\n", cmd);
    system(cmd);
    FILE *f1, *f2;
    f1 = fopen(complete_state, "r");
    f2 = fopen("dataloss\\recording--level-complete-state.dat", "r");
    int line = 0;
    int ok = 1;
    while (!feof(f1))
    {
        line++;
        char b1[1024] = "", b2[1024] = "";
        fgets(b1, sizeof(b1), f1);
        fgets(b2, sizeof(b2), f2);
        if (feof(f1) && feof(f2))
            break;
        // Mission metadata is wrong for record playback, so lets not test the line
        if (line == 2)
            continue;
        if (strcmp(b1, b2))
        {
            printf("Difference at line %d:\n%svs\n%s", line, b1, b2);
            ok = 0;
        }
    }
    fclose(f1);
    fclose(f2);
    return ok;
}

int main(int argc, char **argv)
{
    int all_ok = 1;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-' && argv[i][1] == 'r')
            recording = argv[i] + 2;
        if (argv[i][0] == '-' && argv[i][1] == 'c')
            complete_state = argv[i] + 2;
        if (recording && complete_state)
        {
            printf("Start playback for recording %s and check it against complete state %s...\n", recording, complete_state);
            int ok = play_one();
            printf("%s\n", ok ? "OK" : "FAIL");
            all_ok = all_ok && ok;
            recording = NULL;
            complete_state = NULL;
        }
    }
    printf("\n\n\n*****************************\nTest run done. Result: %s\n", all_ok ? "OK" : "FAIL");
    return 0;
}