#include "predictable_random.h"
#include <stdint.h>

#define PREDICTABLE_RANDOM_SEED 8507

static inline uint32_t get_pseudo_random_num(uint32_t state)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

static uint32_t state = 0;

int pr_get_random()
{
    state = get_pseudo_random_num(state);
    return state & 0x7FFFFFFF;
}

void pr_reset_random()
{
    state = PREDICTABLE_RANDOM_SEED;
}
