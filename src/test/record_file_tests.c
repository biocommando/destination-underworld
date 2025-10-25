#include <unittests.h>
#include "../record_file.h"

static const char *fn = "record_file__read_write_and_switching_files.txt";
static const char *fn2 = "record_file__read_write_and_switching_files2.txt";

static void create_test_file()
{
    FILE *f;
    f = fopen(fn, "w");
    fprintf(f, "rec1: \"hello\" \"world\"\nrec2: \"hello\" \"universe\"\n# this is not a record\nrec3: \"hello\" \"mom!\"\n");
    fclose(f);
}

void record_file__read_write_and_switching_files()
{
    create_test_file();
    int ret = record_file_find_and_read(fn, "rec1");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(record_file_next_param(), "hello"));
    ASSERT(STR_EQ(record_file_next_param(), "world"));
    ASSERT(record_file_next_param() == NULL);
    ret = record_file_find_and_read(fn, "rec2");
    ASSERT(STR_EQ(record_file_next_param(), "hello"));
    ASSERT(STR_EQ(record_file_next_param(), "universe"));
    ASSERT(record_file_next_param() == NULL);
    ret = record_file_find_and_read(fn, "rec3");
    ASSERT(STR_EQ(record_file_next_param(), "hello"));
    ASSERT(STR_EQ(record_file_next_param(), "mom!"));
    ASSERT(record_file_next_param() == NULL);

    ret = record_file_find_and_modify(fn2, "hello");
    ASSERT(INT_EQ(0, ret));
    record_file_add_param("world");
    ASSERT(fopen(fn2, "r") == NULL);
    record_file_flush();
    FILE *f = fopen(fn2, "r");
    char s[100];
    int sz = fread(s, 1, 100, f);
    fclose(f);
    s[sz] = 0;
    ASSERT(STR_EQ(s, "hello:\"world\"\n"));

    record_file_find_and_modify(fn, "rec1");
    record_file_add_param("overwriting");
    record_file_add_param("this record");

    ret = record_file_find_and_modify(fn, "rec5");
    ASSERT(INT_EQ(0, ret));
    record_file_add_param("completely new record");
    ret = record_file_find_and_read(fn, "rec1");
    ASSERT(INT_EQ(0, ret));
    ASSERT(STR_EQ(record_file_next_param(), "overwriting"));
    ASSERT(STR_EQ(record_file_next_param(), "this record"));
    ASSERT(record_file_next_param() == NULL);
    ret = record_file_find_and_read(fn, "rec2");
    ASSERT(STR_EQ(record_file_next_param(), "hello"));
    ASSERT(STR_EQ(record_file_next_param(), "universe"));
    ASSERT(record_file_next_param() == NULL);
    ret = record_file_find_and_read(fn, "rec3");
    ASSERT(STR_EQ(record_file_next_param(), "hello"));
    ASSERT(STR_EQ(record_file_next_param(), "mom!"));
    ASSERT(record_file_next_param() == NULL);
    ret = record_file_find_and_read(fn, "rec5");
    ASSERT(STR_EQ(record_file_next_param(), "completely new record"));
    ASSERT(record_file_next_param() == NULL);

    record_file_flush();
    remove(fn);
    remove(fn2);
}

void record_file__format_functions()
{
    create_test_file();

    record_file_find_and_read(fn, "rec1");
    ASSERT(INT_EQ(-1234, record_file_next_param_as_int(-1234)));
    ASSERT(FLOAT_EQ(-1234, record_file_next_param_as_float(-1234)));

    record_file_find_and_modify(fn, "rec1");
    record_file_add_int_param(100);
    float f = 200.12;
    record_file_add_float_param(f);

    record_file_find_and_read(fn, "rec1");
    ASSERT(INT_EQ(100, record_file_next_param_as_int(-1234)));
    ASSERT(FLOAT_EQ(f, record_file_next_param_as_float(-1234)));
    record_file_find_and_read(fn, "rec1");
    ASSERT(STR_EQ(record_file_next_param(), "100"));
    char expected[20];
    sprintf(expected, "%f", f);
    ASSERT(STR_EQ(record_file_next_param(), expected));

    record_file_flush();
    remove(fn);
}

void record_file__limits()
{
    FILE *f;
    f = fopen(fn, "w");
    fprintf(f, "rec1: \"");
    for (int i = 0; i < 2000; i++)
    {
        fprintf(f, "x");
        if (i == 1000)
            fprintf(f, "\" rec2: \"");
    }
    fprintf(f, "\"\n");
    fprintf(f, "rec3: \"hello\"\n");
    fclose(f);

    record_file_find_and_read(fn, "rec3");
    ASSERT(STR_EQ(record_file_next_param(), "hello"));
    record_file_find_and_modify(fn, "rec3");
    record_file_add_param("hello world");
    record_file_flush();
    f = fopen(fn, "r");
    char buf[101];
    int sz = fread(buf, 1, 100, f);
    buf[sz] = 0;
    ASSERT(STR_EQ(buf, "rec3:\"hello world\"\n"));
    fclose(f);
    remove(fn);
}

void record_file__cannot_open_file__does_not_crash()
{
    record_file_find_and_modify("path/that/does/not/exist", "hello");
    record_file_flush();
}

void test_suite__record_file()
{
    RUN_TEST(record_file__read_write_and_switching_files);
    RUN_TEST(record_file__format_functions);
    RUN_TEST(record_file__limits);
    RUN_TEST(record_file__cannot_open_file__does_not_crash);
}