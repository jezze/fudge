#include <fudge.h>
#include "number.h"

#define LN2HI                           6.93147180369123816490e-01
#define LN2LO                           1.90821492927058770002e-10
#define PIO2HI                          1.57079632673412561417e+00
#define PIO2LO                          6.07710050650619224932e-11
#define TWO52                           4503599627370496.0
#define TWO32                           4294967296.0
#define DIGITS                          17

static double zero;

double number_nan(void)
{

    return zero / zero;

}

double number_infinity(void)
{

    return 1.0 / zero;

}

unsigned int number_isnan(double x)
{

    return x != x;

}

unsigned int number_isfinite(double x)
{

    return x == x && x - x == 0.0;

}

double number_floor(double x)
{

    volatile double t;

    if (!number_isfinite(x) || x >= TWO52 || x <= -TWO52)
        return x;

    /* adding and taking away 2^52 drops the fraction, rounding to nearest, which is then corrected downwards */
    t = (x >= 0) ? x + TWO52 : x - TWO52;
    t = (x >= 0) ? t - TWO52 : t + TWO52;

    return (t > x) ? t - 1.0 : t;

}

double number_ceil(double x)
{

    return -number_floor(-x);

}

double number_trunc(double x)
{

    return (x < 0) ? number_ceil(x) : number_floor(x);

}

double number_round(double x)
{

    return number_floor(x + 0.5);

}

double number_mod(double a, double b)
{

    if (number_isnan(a) || number_isnan(b) || !number_isfinite(a) || b == 0)
        return number_nan();

    if (!number_isfinite(b))
        return a;

    return a - number_trunc(a / b) * b;

}

static double power2(int n)
{

    double result = 1.0;
    double base = (n < 0) ? 0.5 : 2.0;
    unsigned int count = (n < 0) ? -n : n;

    while (count)
    {

        if (count & 1)
            result *= base;

        base *= base;
        count >>= 1;

    }

    return result;

}

static double power10(int n)
{

    double result = 1.0;
    double base = 10.0;
    unsigned int count = (n < 0) ? -n : n;

    while (count)
    {

        if (count & 1)
            result *= base;

        base *= base;
        count >>= 1;

    }

    return (n < 0) ? 1.0 / result : result;

}

double number_sqrt(double x)
{

    double scale = 1.0;
    double guess;
    unsigned int i;

    if (number_isnan(x) || x < 0)
        return number_nan();

    if (x == 0 || !number_isfinite(x))
        return x;

    /* bring the value near one by powers of four, whose roots are powers of two */
    while (x >= 4.0)
    {

        x *= 0.25;
        scale *= 2.0;

    }

    while (x < 0.25)
    {

        x *= 4.0;
        scale *= 0.5;

    }

    guess = (x + 1.0) * 0.5;

    for (i = 0; i < 8; i++)
        guess = (guess + x / guess) * 0.5;

    return guess * scale;

}

double number_exp(double x)
{

    double n;
    double r;
    double term = 1.0;
    double sum = 1.0;
    unsigned int i;

    if (number_isnan(x))
        return x;

    if (x > 709.8)
        return number_infinity();

    if (x < -745.2)
        return 0.0;

    /* e^x is 2^n times e^r, with r small enough for a short series */
    n = number_round(x / LN2HI);
    r = (x - n * LN2HI) - n * LN2LO;

    for (i = 1; i < 30; i++)
    {

        term *= r / i;
        sum += term;

        if (term < 1e-17 && term > -1e-17)
            break;

    }

    return sum * power2((int)n);

}

double number_log(double x)
{

    int k = 0;
    double s;
    double s2;
    double term;
    double sum;
    unsigned int i;

    if (number_isnan(x) || x < 0)
        return number_nan();

    if (x == 0)
        return -number_infinity();

    if (!number_isfinite(x))
        return x;

    /* x is m times 2^k with m near one, and log m comes from the series of atanh */
    while (x >= TWO32)
    {

        x /= TWO32;
        k += 32;

    }

    while (x < 1.0 / TWO32)
    {

        x *= TWO32;
        k -= 32;

    }

    while (x >= 2.0)
    {

        x *= 0.5;
        k++;

    }

    while (x < 1.0)
    {

        x *= 2.0;
        k--;

    }

    if (x > 1.4142135623730951)
    {

        x *= 0.5;
        k++;

    }

    s = (x - 1.0) / (x + 1.0);
    s2 = s * s;
    term = s;
    sum = 0;

    for (i = 1; i < 60; i += 2)
    {

        sum += term / i;
        term *= s2;

        if (term < 1e-18 && term > -1e-18)
            break;

    }

    return k * LN2HI + (2.0 * sum + k * LN2LO);

}

double number_pow(double a, double b)
{

    if (b == 0)
        return 1.0;

    if (number_isnan(a) || number_isnan(b))
        return number_nan();

    /* whole powers are multiplied out, which keeps them exact as long as the result fits */
    if (b == number_floor(b) && b < 2147483648.0 && b > -2147483648.0)
    {

        double result = 1.0;
        double base = a;
        unsigned int count = (b < 0) ? (unsigned int)-b : (unsigned int)b;

        while (count)
        {

            if (count & 1)
                result *= base;

            base *= base;
            count >>= 1;

        }

        return (b < 0) ? 1.0 / result : result;

    }

    if (a < 0)
        return number_nan();

    if (a == 0)
        return (b > 0) ? 0.0 : number_infinity();

    return number_exp(b * number_log(a));

}

static double sinseries(double x)
{

    double x2 = x * x;

    return x * (1.0 - x2 / 6.0 * (1.0 - x2 / 20.0 * (1.0 - x2 / 42.0 * (1.0 - x2 / 72.0 * (1.0 - x2 / 110.0 * (1.0 - x2 / 156.0 * (1.0 - x2 / 210.0 * (1.0 - x2 / 272.0))))))));

}

static double cosseries(double x)
{

    double x2 = x * x;

    return 1.0 - x2 / 2.0 * (1.0 - x2 / 12.0 * (1.0 - x2 / 30.0 * (1.0 - x2 / 56.0 * (1.0 - x2 / 90.0 * (1.0 - x2 / 132.0 * (1.0 - x2 / 182.0 * (1.0 - x2 / 240.0)))))));

}

static double quadrant(double x, unsigned int cosine)
{

    double k;
    double r;
    unsigned int q;

    if (!number_isfinite(x))
        return number_nan();

    /* the angle comes down to within an eighth turn of zero, the quarter turns taken off pick the series and sign */
    k = number_round(x / PIO2HI);
    r = (x - k * PIO2HI) - k * PIO2LO;
    q = ((unsigned int)number_mod(number_mod(k, 4.0) + 4.0, 4.0) + cosine) & 3;

    switch (q)
    {

    case 0:
        return sinseries(r);

    case 1:
        return cosseries(r);

    case 2:
        return -sinseries(r);

    }

    return -cosseries(r);

}

double number_sin(double x)
{

    return quadrant(x, 0);

}

double number_cos(double x)
{

    return quadrant(x, 1);

}

double number_tan(double x)
{

    return number_sin(x) / number_cos(x);

}

double number_atan(double x)
{

    double x2;
    double term;
    double sum = 0;
    unsigned int i;

    if (number_isnan(x))
        return x;

    if (x < 0)
        return -number_atan(-x);

    if (x > 1.0)
        return NUMBER_PI / 2 - number_atan(1.0 / x);

    /* above tan(pi/8) the angle is taken from pi/4 so the series stays short */
    if (x > 0.41421356237309503)
        return NUMBER_PI / 4 + number_atan((x - 1.0) / (x + 1.0));

    x2 = x * x;
    term = x;

    for (i = 1; i < 80; i += 2)
    {

        sum += ((i & 2) ? -term : term) / i;
        term *= x2;

        if (term < 1e-18)
            break;

    }

    return sum;

}

double number_atan2(double y, double x)
{

    if (number_isnan(x) || number_isnan(y))
        return number_nan();

    if (x > 0)
        return number_atan(y / x);

    if (x < 0)
        return (y >= 0) ? number_atan(y / x) + NUMBER_PI : number_atan(y / x) - NUMBER_PI;

    if (y > 0)
        return NUMBER_PI / 2;

    if (y < 0)
        return -NUMBER_PI / 2;

    return 0;

}

double number_asin(double x)
{

    if (x > 1.0 || x < -1.0)
        return number_nan();

    return number_atan2(x, number_sqrt(1.0 - x * x));

}

double number_acos(double x)
{

    if (x > 1.0 || x < -1.0)
        return number_nan();

    return number_atan2(number_sqrt(1.0 - x * x), x);

}

int number_toint32(double x)
{

    double m;

    if (!number_isfinite(x))
        return 0;

    /* the whole part taken modulo 2^32, then read as signed */
    m = number_trunc(x);
    m = m - number_floor(m / TWO32) * TWO32;

    if (m >= TWO32 / 2)
        m -= TWO32;

    return (int)m;

}

struct number_parsed number_parse(char *text, unsigned int count)
{

    struct number_parsed parsed;
    double mantissa = 0;
    int exponent = 0;
    unsigned int digits = 0;
    unsigned int significant = 0;
    unsigned int negative = 0;
    unsigned int i = 0;

    if (i < count && (text[i] == '-' || text[i] == '+'))
    {

        negative = text[i] == '-';
        i++;

    }

    if (count - i >= 8 && buffer_match(text + i, "Infinity", 8))
    {

        parsed.value = (negative) ? -number_infinity() : number_infinity();
        parsed.used = i + 8;

        return parsed;

    }

    /* digits past those a double can hold only move the decimal point */
    for (; i < count && text[i] >= '0' && text[i] <= '9'; i++, digits++)
    {

        if (significant < 19)
        {

            mantissa = mantissa * 10 + (text[i] - '0');
            significant += (mantissa > 0);

        }

        else
        {

            exponent++;

        }

    }

    if (i < count && text[i] == '.')
    {

        for (i++; i < count && text[i] >= '0' && text[i] <= '9'; i++, digits++)
        {

            if (significant < 19)
            {

                mantissa = mantissa * 10 + (text[i] - '0');
                significant += (mantissa > 0);
                exponent--;

            }

        }

    }

    if (!digits)
    {

        parsed.value = number_nan();
        parsed.used = 0;

        return parsed;

    }

    if (i < count && (text[i] == 'e' || text[i] == 'E'))
    {

        unsigned int start = i;
        unsigned int negativeexponent = 0;
        int value = 0;

        i++;

        if (i < count && (text[i] == '-' || text[i] == '+'))
        {

            negativeexponent = text[i] == '-';
            i++;

        }

        if (i < count && text[i] >= '0' && text[i] <= '9')
        {

            for (; i < count && text[i] >= '0' && text[i] <= '9'; i++)
                value = (value < 10000) ? value * 10 + (text[i] - '0') : value;

            exponent += (negativeexponent) ? -value : value;

        }

        else
        {

            i = start;

        }

    }

    /* a large power of ten is split so neither half overflows before the other is applied */
    if (exponent < -300)
        mantissa = mantissa * power10(-300) * power10(exponent + 300);
    else if (exponent > 300)
        mantissa = mantissa * power10(300) * power10(exponent - 300);
    else if (exponent < 0)
        mantissa = mantissa / power10(-exponent);
    else
        mantissa = mantissa * power10(exponent);

    parsed.value = (negative) ? -mantissa : mantissa;
    parsed.used = i;

    return parsed;

}

static unsigned int writeinteger(double value, char *buffer)
{

    char digits[NUMBER_TEXTSIZE];
    unsigned int count = 0;
    unsigned int i;

    do
    {

        double next = number_floor(value / 10.0);

        digits[count++] = '0' + (int)(value - next * 10.0);
        value = next;

    } while (value >= 1.0 && count < NUMBER_TEXTSIZE - 1);

    for (i = 0; i < count; i++)
        buffer[i] = digits[count - 1 - i];

    return count;

}

struct digits
{

    unsigned int count;
    int point;

};

static struct digits shortest(double value, char *digits)
{

    int exponent = (int)number_floor(number_log(value) / number_log(10.0));
    unsigned int precision;
    struct digits result;

    result.count = 0;
    result.point = 0;

    if (power10(exponent) > value)
        exponent--;

    if (power10(exponent + 1) <= value)
        exponent++;

    /* the fewest digits that read back as the same number */
    for (precision = 1; precision <= DIGITS; precision++)
    {

        int shift = (int)precision - 1 - exponent;
        double scaled = number_round((shift >= 0) ? value * power10(shift) : value / power10(-shift));
        int top = exponent;
        char text[NUMBER_TEXTSIZE + 8];
        unsigned int count;
        unsigned int length;
        int e;

        if (scaled >= power10((int)precision))
        {

            scaled = number_round(scaled / 10.0);
            top++;

        }

        count = writeinteger(scaled, digits);
        e = top + 1 - (int)count;
        length = count;

        buffer_copy(text, digits, count);

        text[length++] = 'e';
        text[length++] = (e < 0) ? '-' : '+';
        length += writeinteger((e < 0) ? -e : e, text + length);

        if (number_parse(text, length).value == value || precision == DIGITS)
        {

            while (count > 1 && digits[count - 1] == '0')
                count--;

            result.count = count;
            result.point = top + 1;

            break;

        }

    }

    return result;

}

unsigned int number_format(double value, char *buffer)
{

    char digits[NUMBER_TEXTSIZE];
    unsigned int offset = 0;
    struct digits found;
    unsigned int count;
    int n;
    unsigned int i;

    if (number_isnan(value))
    {

        buffer_copy(buffer, "NaN", 4);

        return 3;

    }

    if (value == 0)
    {

        buffer_copy(buffer, "0", 2);

        return 1;

    }

    if (value < 0)
    {

        buffer[offset++] = '-';
        value = -value;

    }

    if (!number_isfinite(value))
    {

        buffer_copy(buffer + offset, "Infinity", 9);

        return offset + 8;

    }

    found = shortest(value, digits);
    count = found.count;
    n = found.point;

    /* where the point goes follows the rules of Number.prototype.toString */
    if ((int)count <= n && n <= 21)
    {

        for (i = 0; i < count; i++)
            buffer[offset++] = digits[i];

        for (i = count; (int)i < n; i++)
            buffer[offset++] = '0';

    }

    else if (n > 0 && n <= 21)
    {

        for (i = 0; i < count; i++)
        {

            if ((int)i == n)
                buffer[offset++] = '.';

            buffer[offset++] = digits[i];

        }

    }

    else if (n > -6 && n <= 0)
    {

        buffer[offset++] = '0';
        buffer[offset++] = '.';

        for (i = 0; (int)i < -n; i++)
            buffer[offset++] = '0';

        for (i = 0; i < count; i++)
            buffer[offset++] = digits[i];

    }

    else
    {

        buffer[offset++] = digits[0];

        if (count > 1)
            buffer[offset++] = '.';

        for (i = 1; i < count; i++)
            buffer[offset++] = digits[i];

        buffer[offset++] = 'e';
        buffer[offset++] = (n - 1 < 0) ? '-' : '+';
        offset += writeinteger((n - 1 < 0) ? -(n - 1) : n - 1, buffer + offset);

    }

    buffer[offset] = '\0';

    return offset;

}

unsigned int number_fixed(double value, unsigned int digits, char *buffer)
{

    char whole[NUMBER_TEXTSIZE];
    unsigned int offset = 0;
    unsigned int count;
    double scaled;
    unsigned int i;

    if (!number_isfinite(value) || value >= 1e21 || value <= -1e21)
        return number_format(value, buffer);

    if (digits > 20)
        digits = 20;

    /* the nearest number with that many decimals, the larger one when two are as near */
    scaled = number_floor(((value < 0) ? -value : value) * power10((int)digits) + 0.5);

    if (value < 0 && scaled > 0)
        buffer[offset++] = '-';

    count = writeinteger(scaled, whole);

    for (i = count; i <= digits; i++)
        buffer[offset++] = '0';

    for (i = 0; i < count; i++)
    {

        if (digits && count - i == digits)
            buffer[offset++] = '.';

        buffer[offset++] = whole[i];

    }

    /* writing the zeros in front already placed the point when the number had fewer digits than decimals */
    if (digits && count <= digits)
    {

        unsigned int start = (value < 0 && scaled > 0) ? 1 : 0;

        for (i = offset; i > start + 1; i--)
            buffer[i] = buffer[i - 1];

        buffer[start + 1] = '.';
        offset++;

    }

    buffer[offset] = '\0';

    return offset;

}

unsigned int number_radix(double value, unsigned int radix, char *buffer)
{

    char digits[80];
    unsigned int offset = 0;
    unsigned int count = 0;
    double whole;
    double fraction;
    unsigned int i;

    if (radix == 10 || radix < 2 || radix > 36 || !number_isfinite(value))
        return number_format(value, buffer);

    if (value < 0)
    {

        buffer[offset++] = '-';
        value = -value;

    }

    whole = number_floor(value);
    fraction = value - whole;

    do
    {

        double next = number_floor(whole / radix);
        unsigned int digit = (unsigned int)(whole - next * radix);

        digits[count++] = (digit < 10) ? '0' + digit : 'a' + digit - 10;
        whole = next;

    } while (whole >= 1.0 && count < 64);

    for (i = 0; i < count; i++)
        buffer[offset++] = digits[count - 1 - i];

    /* the fraction goes on until it runs out or there are more digits than a double can mean */
    if (fraction > 0)
    {

        buffer[offset++] = '.';

        for (i = 0; i < 52 && fraction > 0; i++)
        {

            unsigned int digit;

            fraction *= radix;
            digit = (unsigned int)fraction;
            fraction -= digit;
            buffer[offset++] = (digit < 10) ? '0' + digit : 'a' + digit - 10;

        }

    }

    buffer[offset] = '\0';

    return offset;

}

