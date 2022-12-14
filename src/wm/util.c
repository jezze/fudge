#include <fudge.h>
#include <abi.h>
#include "util.h"

int util_intersects(int v, int min, int max)
{

    return v >= min && v < max;

}

int util_min(int a, int b)
{

    return (a < b) ? a : b;

}

int util_max(int a, int b)
{

    return (a > b) ? a : b;

}

int util_clamp(int v, int min, int max)
{

    if (v < min)
        v = min;

    if (v > max)
        v = max;

    return v;

}

