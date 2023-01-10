#include <fudge.h>
#include <math.h>
#include "types.h"
#include "vector2.h"

struct vector2 vector2_create(double x, double y)
{

    struct vector2 n;

    n.x = x;
    n.y = y;

    return n;

}

struct vector2 vector2_zero(void)
{

    return vector2_create(0.0, 0.0);

}

struct vector2 vector2_add(struct vector2 *v, double x, double y)
{

    struct vector2 n;

    n.x = v->x + x;
    n.y = v->y + y;

    return n;

}

struct vector2 vector2_add_vector2(struct vector2 *v1, struct vector2 *v2)
{

    return vector2_add(v1, v2->x, v2->y);

}

struct vector2 vector2_mul(struct vector2 *v, double x, double y)
{

    struct vector2 n;

    n.x = v->x * x;
    n.y = v->y * y;

    return n;

}

struct vector2 vector2_mul_vector2(struct vector2 *v1, struct vector2 *v2)
{

    return vector2_mul(v1, v2->x, v2->y);

}

