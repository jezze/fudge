#include "math.h"

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

double math_mod(double x, double y)
{

    return x - math_trunc(x / y) * y;

}

double math_sqrt(double x)
{

    double z = 1.0;
    unsigned int i;

    for (i = 0; i < 10; i++)
        z -= (z * z - x) / (2 * z);

    return z;

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

    double y = x;
    double s = -1;
    unsigned int i;

    for (i = 3; i <= 100; i += 2)
    {

        y += s * (math_pow(x, i) / math_fact(i));
        s *= -1;

    }

    return y;

}

double math_cos(double x)
{

    double y = 1;
    double s = -1;
    unsigned int i;

    for (i = 2; i <= 100; i += 2)
    {

        y += s * (math_pow(x, i) / math_fact(i));
        s *= -1;

    }

    return y;

}

double math_tan(double x)
{

    return (math_sin(x) / math_cos(x));

}

