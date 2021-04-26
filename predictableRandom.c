#include"predictableRandom.h"

uint32_t pr_get_random_stateless(uint32_t state)
{
	state ^= state << 13;
	state ^= state >> 17;
	state ^= state << 5;
	return state;
}

int predictable_random_next(int reset)
{
    static uint32_t state = 0;
    if (reset)
    {
        state = PREDICTABLE_RANDOM_SEED;
        return 0;
    }
    state = pr_get_random_stateless(state);
    return state & 0x7FFFFFFF;
}


int pr_get_random()
{
    return predictable_random_next(0);
}

void pr_reset_random()
{
    predictable_random_next(1);
}
