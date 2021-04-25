#ifndef DUHELPERS_H
#define DUHELPERS_H

inline int imin(int a, int b);
inline int imax(int a, int b);
inline void ilimit(int *v, int limit);
inline void climit(char *v, char limit);

void chunkrest(int ms);

typedef struct coordinates {
    double x;
    double y;
} Coordinates;

#endif
