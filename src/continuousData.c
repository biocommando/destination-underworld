#include "continuousData.h"

int read_continuous_data_buffer(ContinuousData *buffer, int sz, FILE *f)
{
    char s[256];
    int i = 0;
    while (!feof(f) && i < sz)
    {
        fgets(s, 256, f);
        int a;
        long b, c;
        int read_values = sscanf(s, "%d@%lu:%lu", &a, &b, &c);
        if (read_values == 3)
        {
            buffer[i].data_type_id = a;
            buffer[i].time_stamp = b;
            buffer[i].data_value = c;
            i++;
        }
    }
    return i;
}

void write_continuous_data_buffer(ContinuousData *buffer, int sz, FILE *f)
{
    for (int i = 0; i < sz; i++)
    {
        ContinuousData *d = buffer + i;
        fprintf(f, "%d@%u:%u\n", d->data_type_id, d->time_stamp, d->data_value);
    }
}

void produce_and_write_continuous_data(ContinuousData *buffer, int sz, int *idx, FILE *f, long time_stamp, int id, long value)
{
    buffer[*idx].data_type_id = id;
    buffer[*idx].time_stamp = time_stamp;
    buffer[*idx].data_value = value;
    *idx = *idx + 1;
    if (*idx == sz)
    {
        write_continuous_data_buffer(buffer, sz, f);
        *idx = 0;
    }
}

int read_and_process_continuous_data(ContinuousData *buffer, int *sz, int *idx, FILE *f, long time_stamp,
                                     void (*callback)(ContinuousData *, void *), void *callback_arg)
{
    if (*idx == -1)
    {
        *sz = read_continuous_data_buffer(buffer, *sz, f);
        if (*sz == 0)
            return 0;
        *idx = 0;
    }
    while (buffer[*idx].time_stamp == time_stamp)
    {
        callback(&buffer[*idx], callback_arg);
        *idx = *idx + 1;
        if (*idx == *sz)
        {
            *sz = read_continuous_data_buffer(buffer, *sz, f);
            if (*sz == 0)
                return 0;
            *idx = 0;
            return read_and_process_continuous_data(buffer, sz, idx, f, time_stamp, callback, callback_arg);
        }
    }
    return 1;
}
