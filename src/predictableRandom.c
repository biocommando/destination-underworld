#include "predictableRandom.h"
#include <stdint.h>

#define PREDICTABLE_RANDOM_SEED 8507

static inline uint32_t get_pseudo_random_num(uint32_t state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

static inline int predictable_random_next(int reset)
{
    static uint32_t state = 0;
    if (reset)
    {
        state = PREDICTABLE_RANDOM_SEED;
        return 0;
    }
    state = get_pseudo_random_num(state);
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
