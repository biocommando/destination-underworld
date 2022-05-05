#ifndef DUHELPERS_H
#define DUHELPERS_H
#include <time.h>

inline int imin(int a, int b);
inline int imax(int a, int b);
inline void ilimit(int *v, int limit);
inline void climit(char *v, char limit);

void chunkrest(int ms);

void game_loop_rest(clock_t *state);

typedef struct coordinates
{
    double x;
    double y;
} Coordinates;

#endif
