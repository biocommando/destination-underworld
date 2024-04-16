#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int record_file_read_write_record(FILE *src, FILE *dst, const char *id, char *record, size_t sz)
{
    char line[1024], key[1024];
    while (fgets(line, sizeof(line), src))
    {
        sscanf(line, "%s", key);
        if (!strcmp(key, id))
        {
            if (!dst)
            {
                memcpy(record, line, sz > sizeof(line) ? sizeof(line) : sz);
                return 0;
            }
            else
            {
                fprintf(dst, "%s\n", record);
            }
        }
        else if (dst)
        {
            fprintf(dst, "%s", line);
        }
    }
    return dst ? 0 : 1;
}


int record_file_get_record(const char *file, const char *id, char *record, size_t sz)
{
    FILE *src = fopen(file, "r");
    if (!src)
    {
        return 1;
    }
    int ret = record_file_read_write_record(src, NULL, id, record, sz);
    fclose(src);
    return ret;
}

int record_file_set_record(const char *file, const char *id, const char *record)
{
    FILE *src = fopen(file, "r");
    if (!src)
    {
        return 1;
    }
    FILE *dst = fopen("temp", "w");
    if (!dst)
    {
        fclose(src);
        return 1;
    }
    int ret = record_file_read_write_record(src, dst, id, record, 0);
    fclose(src);
    fclose(dst);
    remove(file);
    rename("temp", file);
    return ret;
}