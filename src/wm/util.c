#include <fudge.h>
#include <abi.h>
#include "util.h"

unsigned int util_intersects(int v, int min, int max)
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
        return min;

    if (v > max)
        return max;

    return v;

}

struct util_position util_position(int x, int y)
{

    struct util_position position;

    position.x = x;
    position.y = y;

    return position;

}

struct util_size util_size(int w, int h)
{

    struct util_size size;

    size.w = w;
    size.h = h;

    return size;

}

struct util_size util_size_intersection(struct util_size *a, struct util_size *b)
{

    struct util_size result;

    result.w = util_min(a->w, b->w);
    result.h = util_min(a->h, b->h);

    return result;

}

struct util_region util_region(int x, int y, unsigned int w, unsigned int h)
{

    struct util_region region;

    region.position = util_position(x, y);
    region.size = util_size(w, h);

    return region;

}

unsigned int util_region_intersects(struct util_region *region, int x, int y)
{

    return util_intersects(x, region->position.x, region->position.x + region->size.w) && util_intersects(y, region->position.y, region->position.y + region->size.h);

}

struct util_region util_region_intersection(struct util_region *a, struct util_region *b)
{

    struct util_region result;

    result.position.x = util_max(a->position.x, b->position.x);
    result.position.y = util_max(a->position.y, b->position.y);
    result.size.w = util_min(a->position.x + a->size.w, b->position.x + b->size.w) - result.position.x;
    result.size.h = util_min(a->position.y + a->size.h, b->position.y + b->size.h) - result.position.y;

    return result;

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

