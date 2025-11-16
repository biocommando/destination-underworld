#include <stdlib.h>
#include "helpers.h"
#include "allegro_management.h"
#include "game_tuning.h"

void game_loop_rest(double *state)
{
    while (al_get_time() - *state < 0.025)
    {
        wait_delay(1);
        consume_event_queue();
    }
    *state = al_get_time();
}

double random()
{
    return (double)(rand() % 1000) / 1000;
}

int calculate_next_perk_xp(int perks)
{
    const GameTuningParams *gt = get_tuning_params();
    double xp = gt->perk_xp_base;
    for (int i = 0; i < 32; i++)
    {
        if ((perks >> i) & 1)
            xp *= gt->perk_xp_level_multiplier;
    }
    return (int)xp;
}
