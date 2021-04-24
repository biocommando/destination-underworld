#ifndef PREDICTABLE_H
#define PREDICTABLE_H
#include<stdint.h>

#define PREDICTABLE_RANDOM_SEED 8507

int prGetRandom();
uint32_t prGetRandomStateless(uint32_t state);
void prResetRandom();

#endif
