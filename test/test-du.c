#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *recording = NULL, *complete_state = NULL;
char cmdl_extra[1024] = "";

int play_one()
{
    char cmd[1024];
    sprintf(cmd, "DestinationUnderworld.exe --record-mode=play --file=%s "
                 "--start-without-user-interaction=1 --core-pack--require-authentication=0 "
                 "--screenshot-buffer=1 %s",
            recording, cmdl_extra);
    printf("Commandline:\n%s\n", cmd);
    system(cmd);
    FILE *f1, *f2;
    f1 = fopen(complete_state, "r");
    f2 = fopen("dataloss/recording--level-complete-state.dat", "r");
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

#define WRITE_TRES(...)                           \
    do                                            \
    {                                             \
        FILE *f = fopen("test-results.txt", "a"); \
        fprintf(f, __VA_ARGS__);                  \
        fclose(f);                                \
    } while (0)

int main(int argc, char **argv)
{
    remove("test-results.txt");
    WRITE_TRES("Start tests with %d arguments\n", argc - 1);
    int num_failed = 0;
    for (int i = 0; i < argc; i++)
    {
        if (argv[i][0] == '-' && argv[i][1] == 'r')
            recording = argv[i] + 2;
        if (argv[i][0] == '-' && argv[i][1] == 'c')
            complete_state = argv[i] + 2;
        if (argv[i][0] == '-' && argv[i][1] == 'x')
            strcpy(cmdl_extra, argv[i] + 2);
        if (recording && complete_state)
        {
            printf("Start playback for recording %s and check it against complete state %s...\n", recording, complete_state);
            WRITE_TRES("Start test run with command line:\n%s \"-x%s\" \"-r%s\" \"-c%s\"\n", argv[0], cmdl_extra, recording, complete_state);
            int ok = play_one();
            WRITE_TRES("    Result: %s\n", ok ? "OK" : "FAIL");
            printf("%s\n", ok ? "OK" : "FAIL");
            if (!ok)
                num_failed++;
            recording = NULL;
            complete_state = NULL;
        }
    }
    printf("\n\n\n*****************************\nTest run done. Result: ");
    if (num_failed == 0)
        printf("OK\n");
    else
        printf("FAIL, number of tests failed: %d\n", num_failed);
    WRITE_TRES("Number of failed tests %d.\n", num_failed);
    return 0;
}