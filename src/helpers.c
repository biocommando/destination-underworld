#include "helpers.h"
#include "allegro42_compat.h"

void game_loop_rest(clock_t *state)
{
    while (clock() - *state < 25)
        wait_delay(1);
    *state = clock();
}
