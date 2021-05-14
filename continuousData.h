#ifndef CONTINUOUSDATA_H
#define CONTINUOUSDATA_H
#include <stdio.h>

typedef struct
{
    int data_type_id;
    long time_stamp;
    long data_value;
} ContinuousData;

void produce_and_write_continuous_data(ContinuousData *buffer, int sz, int *idx, FILE *f, long time_stamp, int id, long value);
int read_and_process_continuous_data(ContinuousData *buffer, int *sz, int *idx, FILE *f, long time_stamp,
                                     void (*callback)(ContinuousData *, void *), void *callback_arg);

#endif
