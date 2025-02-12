#include <stdlib.h>
#include "helpers.h"
#include "allegro_management.h"

void game_loop_rest(clock_t *state)
{
    while (clock() - *state < 25)
        wait_delay(1);
    *state = clock();
}

double random()
{
    return (double)(rand() % 1000) / 1000;
}

int calculate_next_perk_xp(int perks)
{
    int xp = 300;
    for (int i = 0; i < 32; i++)
    {
        if ((perks >> i) & 1)
            xp *= 2;
    }
    return xp;
}