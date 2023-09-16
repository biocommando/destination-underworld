#pragma once

#include <stdint.h>

#define PREDICTABLE_RANDOM_SEED 8507

int pr_get_random();
uint32_t pr_get_random_stateless(uint32_t state);
void pr_reset_random();
