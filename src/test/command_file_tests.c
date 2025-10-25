#include <unittests.h>
#include <stdio.h>
#include <string.h>
#include "../command_file/command_file.h"

static void dispatch(command_file_DispatchDto *dto)
{
    char *buf = (char *)dto->state;
    if (dto->parameters[1])
    {
        if (!strcmp(dto->command, ""))
        {
            if (!strcmp(dto->parameters[1], "upper"))
            {
                int idx;
                sscanf(dto->parameters[0], "%d", &idx);;
                buf[idx] &= ~32;
            }
            return;
        }
        else if (!strcmp(dto->command, "set"))
        {
            char c = *dto->parameters[0];
            int idx;
            sscanf(dto->parameters[1], "%d", &idx);
            buf[idx] = c;
            return;
        }
        else if (!strcmp(dto->command, "increment"))
        {
            int idx, amount;
            sscanf(dto->parameters[0], "%d", &idx);
            sscanf(dto->parameters[1], "%d", &amount);
            buf[idx] += amount;
            return;
        }
        else if (!strcmp(dto->command, "jump if equal"))
        {
            if (!strcmp(dto->parameters[0], buf))
            {
                strcpy(dto->skip_label, dto->parameters[1]);
            }
            return;
        }
        else if (!strcmp(dto->command, ":inc_at_6:")) {
            buf[6]++;
            return;
        }
    }
    strcpy(buf, "1234567");
}

void command_file__smoke_test()
{
    char buf[8] = "7654321";
    FILE *f = fopen("testfile.txt", "w");
    fprintf(f, "this should set to 1234567\n");
    fprintf(f, "set: \"h\" ignore \"1\"\n");
    fprintf(f, "\"1\" \"upper\"\n");
    fprintf(f, "asd \"do\" ddsad \"nothing\"\n");
    fprintf(f, "set: \"a\"\"2\"\n");
    // Let's add a really long line in between
    for (int i = 0; i < 50; i++)
        fprintf(f, "set:\"X\"\"2\" ");
    fprintf(f, "\n");
    fprintf(f, "increment: \"2\" \"4\"\n");
    fprintf(f, "# comment!\n");
    fprintf(f, "set:\"l\"\"3\"\n");
    fprintf(f, "set:\"m\"  \"4\"\n");
    fprintf(f, ":just some label for fun\n");
    fprintf(f, "increment: \"4\" \"-1\"\n");
    fprintf(f, "set:\"o\"  \"5\"\n");
    fprintf(f, "\\.inc_at_6\\.: \"\"\"\"\n");
    fprintf(f, "jump if equal:\"1Hello8\"  \"skip some lines\"\n");
    fprintf(f, "set: \"E\" \"1\"\n");
    fprintf(f, "set: \"R\" \"2\"\n");
    fprintf(f, "set: \"R\" \"3\"\n");
    fprintf(f, "set: \"O\" \"4\"\n");
    fprintf(f, "set: \"R\" \"5\"\n");
    fprintf(f, ":skip some lines\n");
    fprintf(f, "set: \"\\'\" \"0\"\n");
    fprintf(f, "set: \"\\\\\" \"6\"\n");
    fclose(f);

    read_command_file("testfile.txt", dispatch, buf);
    ASSERT(INT_EQ(buf[7], 0));
    ASSERT(STR_EQ(buf, "\"Hello\\"));

    remove("testfile.txt");
}

void test_suite__command_file()
{
    RUN_TEST(command_file__smoke_test);
}
