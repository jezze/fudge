#include <fudge.h>
#include <abi.h>
#include "util.h"

unsigned int util_intersects(int v, int min, int max)
{

    return v >= min && v < max;

}

unsigned int util_intersects_region(struct util_region *region, int x, int y)
{

    return util_intersects(x, region->x, region->x + region->w) && util_intersects(y, region->y, region->y + region->h);

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

void util_initposition(struct util_position *position, int x, int y)
{

    position->x = x;
    position->y = y;

}

void util_initsize(struct util_size *size, int w, int h)
{

    size->w = w;
    size->h = h;

}

void util_initregion(struct util_region *region, int x, int y, unsigned int w, unsigned int h)
{

    region->x = x;
    region->y = y;
    region->w = w;
    region->h = h;

}

unsigned int util_getkey(struct util_token *tokens, unsigned int n, char *value)
{

    unsigned int i;

    for (i = 0; i < n; i++)
    {

        if (cstring_match(tokens[i].value, value))
            return tokens[i].key;

    }

    return 0;

}

