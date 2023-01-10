#include <fudge.h>
#include <math.h>
#include "types.h"
#include "matrix3x3.h"

struct matrix3x3 matrix3x3_create(double x0, double y0, double z0, double x1, double y1, double z1, double x2, double y2, double z2)
{

    struct matrix3x3 m;

    m.x0 = x0;
    m.y0 = y0;
    m.z0 = z0;
    m.x1 = x1;
    m.y1 = y1;
    m.z1 = z1;
    m.x2 = x2;
    m.y2 = y2;
    m.z2 = z2;

    return m;

}

struct matrix3x3 matrix3x3_zero(void)
{

    return matrix3x3_create(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

}

struct matrix3x3 matrix3x3_identity(void)
{

    return matrix3x3_create(1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);

}

struct matrix3x3 matrix3x3_getrotationx(double s, double c)
{

    return matrix3x3_create(1, 0, 0, 0, c, -s, 0, s, c);

}

struct matrix3x3 matrix3x3_getrotationy(double s, double c)
{

    return matrix3x3_create(c, 0, s, 0, 1, 0, -s, 0, c);

}

struct matrix3x3 matrix3x3_getrotationz(double s, double c)
{

    return matrix3x3_create(c, -s, 0, s, c, 0, 0, 0, 1);

}

