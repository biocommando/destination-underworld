#include"predictableRandom.h"

uint32_t prGetRandomStateless(uint32_t state)
{
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

int predictableRandom_next(int reset)
{
    static uint32_t state = 0;
    if (reset)
    {
        state = PREDICTABLE_RANDOM_SEED;
        return 0;
    }
    state = prGetRandomStateless(state);
    return state & 0x7FFFFFFF;
}


int prGetRandom()
{
    return predictableRandom_next(0);
}

void prResetRandom()
{
    predictableRandom_next(1);
}
