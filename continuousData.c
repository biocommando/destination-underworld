#include "continuousData.h"

int readContinuousDataBuffer(ContinuousData *buffer, int sz, FILE *f)
{
    char s[256];
    int i = 0;
    while(!feof(f) && i < sz)
    {
        fgets(s, 256, f);
        int a;
        long b, c;
        int readValues = sscanf(s, "%d@%lu:%lu", &a, &b, &c);
        if (readValues == 3)
        {
            buffer[i].dataTypeId = a;
            buffer[i].timeStamp = b;
            buffer[i].dataValue = c;
            i++;
        }
    }
    return i;
}

void writeContinuousDataBuffer(ContinuousData *buffer, int sz, FILE *f)
{
    for (int i = 0; i < sz; i++)
    {
        ContinuousData *d = buffer + i;
        fprintf(f, "%d@%u:%u\n", d->dataTypeId, d->timeStamp, d->dataValue);
    }
}

void produceAndWriteContinuousData(ContinuousData *buffer, int sz, int *idx, FILE *f, long timeStamp, int id, long value)
{
    // printf("writing @idx=%d, %d:%d\n", *idx, id, value);
    buffer[*idx].dataTypeId = id;
    buffer[*idx].timeStamp = timeStamp;
    buffer[*idx].dataValue = value;
    *idx = *idx + 1;
    if (*idx == sz)
    {
        writeContinuousDataBuffer(buffer, sz, f);
        *idx = 0;
    }
}

int readAndProcessContinuousData(ContinuousData *buffer, int *sz, int *idx, FILE *f, long timeStamp, 
    void (*callback)(ContinuousData*,void*), void *callbackArg)
{
    if (*idx == -1)
    {
        *sz = readContinuousDataBuffer(buffer, *sz, f);
        if (*sz == 0) return 0;
        *idx = 0;
    }
    while (buffer[*idx].timeStamp == timeStamp)
    {
        callback(&buffer[*idx], callbackArg);
        *idx = *idx + 1;
        if (*idx == *sz)
        {
            *sz = readContinuousDataBuffer(buffer, *sz, f);
            if (*sz == 0) return 0;
            *idx = 0;
            return readAndProcessContinuousData(buffer, sz, idx, f, timeStamp, callback, callbackArg);
        }
    }
    return 1;
}
