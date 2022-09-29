#include "helpers.h"
#include "allegro42_compat.h"
#include "dump3.h"

inline int imin(int a, int b)
{
    return a < b ? a : b;
}

inline int imax(int a, int b)
{
    return a > b ? a : b;
}

inline void ilimit(int *v, int limit)
{
    *v = imin(*v, limit);
}

inline void climit(char *v, char limit)
{
    *v = imin(*v, limit);
}

#define CLOCK_TO_MS(clk) (((double)(clk)*1000) / CLOCKS_PER_SEC)

// TODO: switch to event based

void dynamic_scaling_rest(int ms)
{
    static double scaling = 1;
    clock_t start = clock();
    wait_delay_ms((int)(ms * scaling));
    double spent_ms = CLOCK_TO_MS(clock() - start);
    if (spent_ms != 0 && ms >= 10)
        scaling = (double)ms / spent_ms;
}

void rest_poll(int ms)
{
    clock_t stop = clock() + ms;
    while (clock() < stop)
    {
        wait_delay_ms(1);
    }
}

int chunkrest_step(int ms, int step)
{
    int i;
    for (i = 0; i < ms; i += step)
    {
        play_mp3();
        wait_delay_ms(step);
    }
    return i - ms;
}

void chunkrest(int ms)
{
    if (ms <= 50)
    {
        chunkrest_step(ms, ms);
    }
    else
    {
        int leftover = chunkrest_step(ms, 50);
        if (leftover > 0)
        {
            chunkrest_step(leftover, leftover);
        }
    }
}

void game_loop_rest(clock_t *state)
{
    while (clock() - *state < 25)
        wait_delay(1);
    *state = clock();
}
