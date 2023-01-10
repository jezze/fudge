#include <fudge.h>
#include <math.h>
#include "types.h"
#include "vector3.h"
#include "matrix3x3.h"

struct vector3 vector3_create(double x, double y, double z)
{

    struct vector3 n;

    n.x = x;
    n.y = y;
    n.z = z;

    return n;

}

struct vector3 vector3_zero(void)
{

    return vector3_create(0.0, 0.0, 0.0);

}

struct vector3 vector3_rotatex(struct vector3 *v, double theta)
{

    struct matrix3x3 m = matrix3x3_getrotationx(math_sin(theta), math_cos(theta));

    return vector3_mul_matrix3x3(v, &m);

}

struct vector3 vector3_rotatey(struct vector3 *v, double theta)
{

    struct matrix3x3 m = matrix3x3_getrotationy(math_sin(theta), math_cos(theta));

    return vector3_mul_matrix3x3(v, &m);

}

struct vector3 vector3_rotatez(struct vector3 *v, double theta)
{

    struct matrix3x3 m = matrix3x3_getrotationz(math_sin(theta), math_cos(theta));

    return vector3_mul_matrix3x3(v, &m);

}

struct vector3 vector3_add(struct vector3 *v, double x, double y, double z)
{

    struct vector3 n;

    n.x = v->x + x;
    n.y = v->y + y;
    n.z = v->z + z;

    return n;

}

struct vector3 vector3_add_vector3(struct vector3 *v1, struct vector3 *v2)
{

    struct vector3 n;

    n.x = v1->x + v2->x;
    n.y = v1->y + v2->y;
    n.z = v1->z + v2->z;

    return n;

}

struct vector3 vector3_mul_matrix3x3(struct vector3 *v, struct matrix3x3 *m)
{

    struct vector3 n;

    n.x = v->x * m->x0 + v->y * m->y0 + v->z * m->z0;
    n.y = v->x * m->x1 + v->y * m->y1 + v->z * m->z1;
    n.z = v->x * m->x2 + v->y * m->y2 + v->z * m->z2;

    return n;

}

