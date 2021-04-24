#ifndef CONTINUOUSDATA_H
#define CONTINUOUSDATA_H
#include <stdio.h>

typedef struct {
    int dataTypeId;
    long timeStamp;
    long dataValue;
} ContinuousData;

void produceAndWriteContinuousData(ContinuousData *buffer, int sz, int *idx, FILE *f, long timeStamp, int id, long value);
int readAndProcessContinuousData(ContinuousData *buffer, int *sz, int *idx, FILE *f, long timeStamp, 
    void (*callback)(ContinuousData*,void*), void *callbackArg);

#endif
