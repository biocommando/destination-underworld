#include <unittests.h>
#include "../record_file.h"

static const char *fn = "record_file__read_write_and_switching_files.txt";
static const char *fn2 = "record_file__read_write_and_switching_files2.txt";

static void create_test_file()
{
    FILE *f;
    f = fopen(fn, "w");
    fprintf(f, "rec1 hello world\nrec2 hello universe\n# this is not a record\nrec3 hello mom!\n");
    fclose(f);
}

void record_file__read_write_and_switching_files()
{
    create_test_file();
    char rec[100];
    int ret = record_file_get_record(fn, "rec1", rec, 100);
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(rec, "rec1 hello world"));
    ret = record_file_get_record(fn, "rec2", rec, 100);
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(rec, "rec2 hello universe"));
    ret = record_file_get_record(fn, "rec3", rec, 100);
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(rec, "rec3 hello mom!"));
    ret = record_file_get_record(fn, "#", rec, 100);
    ASSERT(INT_EQ(1, ret));
    ASSERT(STR_EQ(rec, "rec3 hello mom!"));

    record_file_set_record(fn2, "hello", "hello world");
    ASSERT(fopen(fn2, "r") == NULL);
    record_file_flush();
    FILE *f = fopen(fn2, "r");
    int sz = fread(rec, 1, 100, f);
    fclose(f);
    rec[sz] = 0;
    ASSERT(STR_EQ(rec, "hello world\n"));

    record_file_set_record(fn, "rec1", "rec1 overwriting this record");
    ret = record_file_set_record(fn, "rec2", "rec4 overwriting this record with different key");
    ASSERT(INT_EQ(1, ret)); // not allowed
    record_file_set_record(fn, "rec5", "rec5 completely new record");
    ret = record_file_get_record(fn, "rec1", rec, 100);
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(rec, "rec1 overwriting this record"));
    ret = record_file_get_record(fn, "rec2", rec, 100);
    ASSERT(STR_EQ(rec, "rec2 hello universe"));
    ASSERT(INT_EQ(0, ret));
    ret = record_file_get_record(fn, "rec3", rec, 100);
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(rec, "rec3 hello mom!"));
    ret = record_file_get_record(fn, "rec4", rec, 100);
    ASSERT(INT_EQ(1, ret));
    ret = record_file_get_record(fn, "rec5", rec, 100);
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(rec, "rec5 completely new record"));

    // Too small buffer
    ret = record_file_get_record(fn, "rec4", rec, 10);
    ASSERT(INT_EQ(1, ret));

    record_file_flush();
    remove(fn);
    remove(fn2);
}

void record_file__format_functions()
{
    create_test_file();

    char s[4][100];
    int ret = record_file_scanf(fn, "rec1", "%s %s %s %s", s[0], s[1], s[2], s[3]);
    ASSERT(INT_EQ(3, ret));
    ASSERT(STR_EQ("rec1", s[0]));
    ASSERT(STR_EQ("hello", s[1]));
    ASSERT(STR_EQ("world", s[2]));
    record_file_set_record_f(fn, "rec1 %d", 1337);
    record_file_get_record(fn, "rec1", s[0], 100);
    ASSERT(STR_EQ("rec1 1337", s[0]));

    record_file_flush();
    remove(fn);
}

void record_file__limits()
{
    FILE *f;
    f = fopen(fn, "w");
    fprintf(f, "rec1 ");
    for (int i = 0; i < 2000; i++)
        fprintf(f, "x");
    fclose(f);
    char buf[4000];
    memset(buf, 1, 4000);
    int ret = record_file_get_record(fn, "rec1", buf, 4000);
    ASSERT(INT_EQ(0, ret));
    char expected[4000];
    memcpy(expected, "rec1 ", 5);
    memset(expected + 5, 'x', 255 - 5);
    expected[255] = 0;
    memset(expected + 256, 1, 4000 - 256);
    ASSERT(!memcmp(expected, buf, 4000));

    record_file_flush();
    remove(fn);
}

void record_file__cannot_open_file__does_not_crash()
{
    record_file_set_record("path/that/does/not/exist", "hello", "hello world");
    record_file_flush();
}

void test_suite__record_file()
{
    RUN_TEST(record_file__read_write_and_switching_files);
    RUN_TEST(record_file__format_functions);
    RUN_TEST(record_file__limits);
    RUN_TEST(record_file__cannot_open_file__does_not_crash);
}