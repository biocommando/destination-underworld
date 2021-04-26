#include "helpers.h"
#include "allegro.h"
#include "dump3.h"
#include<time.h>

inline int imin(int a, int b) {
    return a < b ? a : b;
}

inline int imax(int a, int b) {
    return a > b ? a : b;
}

inline void ilimit(int *v, int limit) {
    *v = imin(*v, limit);
}

inline void climit(char *v, char limit) {
    *v = imin(*v, limit);
}

#define CLOCK_TO_MS(clk) (((double)(clk) * 1000) / CLOCKS_PER_SEC)

void dynamic_scaling_rest(int ms) {
     static double scaling = 1;
     clock_t start = clock();
     rest((int)(ms * scaling));
     double spent_ms = CLOCK_TO_MS(clock() - start);
     if (spent_ms != 0 && ms >= 10)
       scaling = (double)ms / spent_ms;
}

int chunkrest_step(int ms, int step)
{
 int i;
 for (i = 0; i < ms; i += step)
 {
  play_mp3();
  dynamic_scaling_rest(step);
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
