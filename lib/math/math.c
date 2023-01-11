#include "math.h"

#define INT16_BITS  (16)
#define INT16_MAX   ((1 << (INT16_BITS - 1)) - 1)
#define TABLE_BITS  (5)
#define TABLE_SIZE  (1 << TABLE_BITS)
#define TABLE_MASK  (TABLE_SIZE - 1)
#define LOOKUP_BITS (TABLE_BITS + 2)
#define LOOKUP_MASK ((1 << LOOKUP_BITS) - 1)
#define FLIP_BIT    (1 << TABLE_BITS)
#define NEGATE_BIT  (1 << (TABLE_BITS + 1))
#define INTERP_BITS (INT16_BITS - 1 - LOOKUP_BITS)
#define INTERP_MASK ((1 << INTERP_BITS) - 1)
#define Q15         (1.0 / (double)((1 << 15) - 1))

static short sin90[TABLE_SIZE + 1] = {
    0x0000, 0x0647, 0x0c8b, 0x12c7, 0x18f8, 0x1f19, 0x2527, 0x2b1e,
    0x30fb, 0x36b9, 0x3c56, 0x41cd, 0x471c, 0x4c3f, 0x5133, 0x55f4,
    0x5a81, 0x5ed6, 0x62f1, 0x66ce, 0x6a6c, 0x6dc9, 0x70e1, 0x73b5,
    0x7640, 0x7883, 0x7a7c, 0x7c29, 0x7d89, 0x7e9c, 0x7f61, 0x7fd7,
    0x7fff
};

short math_sin16(short angle)
{

    short v0, v1;

    if (angle < 0)
    {

        angle += INT16_MAX;
        angle += 1;

    }

    v0 = (angle >> INTERP_BITS);

    if (v0 & FLIP_BIT)
    {

        v0 = ~v0;
        v1 = ~angle;

    }

    else
    {

        v1 = angle;

    }

    v0 &= TABLE_MASK;
    v1 = sin90[v0] + (short)(((int)(sin90[v0 + 1] - sin90[v0]) * (v1 & INTERP_MASK)) >> INTERP_BITS);

    if ((angle >> INTERP_BITS) & NEGATE_BIT)
        v1 = -v1;

    return v1;

}

short math_cos16(short angle)
{

    if (angle < 0)
    {

        angle += INT16_MAX;
        angle += 1;

    }

    return math_sin16(angle - (short)(((int)INT16_MAX * 270) / 360));

}

unsigned short math_sqrt16(unsigned short value)
{

    unsigned short root = 0;
    unsigned short bit;

    for (bit = 0x4000; bit > 0; bit >>= 2)
    {

        unsigned short trial = root + bit;

        root >>= 1;

        if (trial <= value)
        {

            root += bit;
            value -= trial;

        }

    }

    return root;

}

double math_deg2rad(double angle)
{

    return angle * (MATH_PI / 180.0);

}

double math_deg2turn(double angle)
{

    return angle / 360.0;

}

double math_rad2deg(double angle)
{

    return angle * (180.0 / MATH_PI);

}

double math_rad2turn(double angle)
{

    return angle / (2.0 * MATH_PI);

}

double math_max(double a, double b)
{

    return a > b ? a : b;

}

double math_min(double a, double b)
{

    return a < b ? a : b;

}

double math_abs(double x)
{

    return x > 0 ? x : -x;

}

double math_floor(double x)
{

    int a = x;

    return a;

}

double math_ceil(double x)
{

    int a = x;

    if ((double)a != x)
        return a + 1;

    return a;

}

double math_trunc(double x)
{

    return (x > 0) ? math_floor(x) : math_ceil(x);

}

double math_mod(double a, double b)
{

    return a - math_trunc(a / b) * b;

}

double math_sqrt(double x)
{

    return math_sqrt16(x);

}

double math_pow(double a, double b)
{

    double c = 1;
    unsigned int i;

    for (i = 0; i < b; i++)
        c *= a;

    return c;

}

double math_fact(double x)
{

    double ret = 1;
    unsigned int i;

    for (i = 1; i <= x; i++)
        ret *= i;

    return ret;

}

double math_sin(double x)
{

    return math_sin16(math_rad2turn(x) * 32768.0) * Q15;

}

double math_cos(double x)
{

    return math_cos16(math_rad2turn(x) * 32768.0) * Q15;

}

double math_tan(double x)
{

    return (math_sin(x) / math_cos(x));

}

