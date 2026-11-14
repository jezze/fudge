__extension__ typedef unsigned long long ulonglong;

#define TOP                             ((ulonglong)1 << 62)
#define CLASS_ZERO                      0
#define CLASS_NORMAL                    1
#define CLASS_INFINITY                  2
#define CLASS_NAN                       3
#define DOUBLE_NAN                      ((ulonglong)0x7FF8 << 48)
#define FLOAT_NAN                       0x7FC00000

struct format
{

    unsigned int fraction;
    int bias;
    unsigned int max;

};

/* a normal number is mantissa * 2 to the power (exponent - 62), with the mantissa's leading 1 at bit 62 */
struct number
{

    unsigned int class;
    unsigned int sign;
    int exponent;
    ulonglong mantissa;

};

static struct format formatdouble = {52, 1023, 0x7FF};
static struct format formatfloat = {23, 127, 0xFF};

static ulonglong shiftright(ulonglong value, unsigned int count)
{

    if (count > 63)
        return (value) ? 1 : 0;

    return (value >> count) | ((value & (((ulonglong)1 << count) - 1)) ? 1 : 0);

}

static void normalize(struct number *number)
{

    if (!number->mantissa)
    {

        number->class = CLASS_ZERO;

        return;

    }

    if (number->mantissa >> 63)
    {

        number->mantissa = shiftright(number->mantissa, 1);
        number->exponent++;

    }

    while (!(number->mantissa & TOP))
    {

        number->mantissa <<= 1;
        number->exponent--;

    }

}

static void unpack(struct number *number, ulonglong bits, struct format *format)
{

    unsigned int exponent = (bits >> format->fraction) & format->max;
    ulonglong fraction = bits & (((ulonglong)1 << format->fraction) - 1);

    number->sign = (bits >> (format->fraction + ((format->max == 0xFF) ? 8 : 11))) & 1;
    number->class = CLASS_NORMAL;

    if (exponent == format->max)
    {

        number->class = (fraction) ? CLASS_NAN : CLASS_INFINITY;

    }

    else if (!exponent)
    {

        number->exponent = 1 - format->bias;
        number->mantissa = fraction << (62 - format->fraction);

        normalize(number);

    }

    else
    {

        number->exponent = exponent - format->bias;
        number->mantissa = (fraction | ((ulonglong)1 << format->fraction)) << (62 - format->fraction);

    }

}

/* rounds to nearest, ties to even, into the format's bits, going subnormal, zero or infinite where it must */
static ulonglong pack(struct number *number, struct format *format)
{

    ulonglong sign = (ulonglong)number->sign << (format->fraction + ((format->max == 0xFF) ? 8 : 11));
    unsigned int roundbits = 62 - format->fraction;
    ulonglong half = (ulonglong)1 << (roundbits - 1);
    ulonglong mantissa;
    ulonglong rest;
    int biased;

    if (number->class == CLASS_NAN)
        return (format->max == 0xFF) ? FLOAT_NAN : DOUBLE_NAN;

    if (number->class == CLASS_INFINITY)
        return sign | ((ulonglong)format->max << format->fraction);

    if (number->class == CLASS_ZERO)
        return sign;

    normalize(number);

    if (number->class == CLASS_ZERO)
        return sign;

    biased = number->exponent + format->bias;
    mantissa = number->mantissa;

    if (biased <= 0)
    {

        mantissa = shiftright(mantissa, 1 - biased);
        biased = 0;

    }

    rest = mantissa & ((half << 1) - 1);
    mantissa >>= roundbits;

    if (rest > half || (rest == half && (mantissa & 1)))
        mantissa++;

    if (!biased)
        return sign | mantissa;

    if (mantissa >> (format->fraction + 1))
    {

        mantissa >>= 1;
        biased++;

    }

    if (biased >= (int)format->max)
        return sign | ((ulonglong)format->max << format->fraction);

    return sign | ((ulonglong)biased << format->fraction) | (mantissa & (((ulonglong)1 << format->fraction) - 1));

}

static ulonglong add(ulonglong a, ulonglong b)
{

    struct number x;
    struct number y;
    struct number result;
    struct number *large = &x;
    struct number *small = &y;

    unpack(&x, a, &formatdouble);
    unpack(&y, b, &formatdouble);

    if (x.class == CLASS_NAN || y.class == CLASS_NAN || (x.class == CLASS_INFINITY && y.class == CLASS_INFINITY && x.sign != y.sign))
        return DOUBLE_NAN;

    if (x.class == CLASS_ZERO && y.class == CLASS_ZERO)
        return (x.sign && y.sign) ? a : 0;

    if (x.class == CLASS_INFINITY || y.class == CLASS_ZERO)
        return a;

    if (y.class == CLASS_INFINITY || x.class == CLASS_ZERO)
        return b;

    if (x.exponent < y.exponent || (x.exponent == y.exponent && x.mantissa < y.mantissa))
    {

        large = &y;
        small = &x;

    }

    result.class = CLASS_NORMAL;
    result.sign = large->sign;
    result.exponent = large->exponent;
    small->mantissa = shiftright(small->mantissa, large->exponent - small->exponent);

    if (large->sign == small->sign)
    {

        result.mantissa = large->mantissa + small->mantissa;

    }

    else
    {

        result.mantissa = large->mantissa - small->mantissa;

        if (!result.mantissa)
            return 0;

    }

    return pack(&result, &formatdouble);

}

static ulonglong multiply(ulonglong a, ulonglong b)
{

    struct number x;
    struct number y;
    struct number result;
    ulonglong a0;
    ulonglong a1;
    ulonglong b0;
    ulonglong b1;
    ulonglong p01;
    ulonglong p10;
    ulonglong middle;
    ulonglong low;
    ulonglong high;

    unpack(&x, a, &formatdouble);
    unpack(&y, b, &formatdouble);

    result.sign = x.sign ^ y.sign;

    if (x.class == CLASS_NAN || y.class == CLASS_NAN || (x.class == CLASS_INFINITY && y.class == CLASS_ZERO) || (x.class == CLASS_ZERO && y.class == CLASS_INFINITY))
        return DOUBLE_NAN;

    if (x.class == CLASS_INFINITY || y.class == CLASS_INFINITY || x.class == CLASS_ZERO || y.class == CLASS_ZERO)
    {

        result.class = (x.class == CLASS_INFINITY || y.class == CLASS_INFINITY) ? CLASS_INFINITY : CLASS_ZERO;

        return pack(&result, &formatdouble);

    }

    x.mantissa >>= 10;
    y.mantissa >>= 10;
    a0 = x.mantissa & 0xFFFFFFFF;
    a1 = x.mantissa >> 32;
    b0 = y.mantissa & 0xFFFFFFFF;
    b1 = y.mantissa >> 32;
    p01 = a0 * b1;
    p10 = a1 * b0;
    middle = ((a0 * b0) >> 32) + (p01 & 0xFFFFFFFF) + (p10 & 0xFFFFFFFF);
    low = (middle << 32) | ((a0 * b0) & 0xFFFFFFFF);
    high = a1 * b1 + (p01 >> 32) + (p10 >> 32) + (middle >> 32);
    result.class = CLASS_NORMAL;
    result.exponent = x.exponent + y.exponent;
    result.mantissa = (high << 22) | (low >> 42) | ((low & (((ulonglong)1 << 42) - 1)) ? 1 : 0);

    return pack(&result, &formatdouble);

}

static ulonglong divide(ulonglong a, ulonglong b)
{

    struct number x;
    struct number y;
    struct number result;
    ulonglong remainder;
    unsigned int i;

    unpack(&x, a, &formatdouble);
    unpack(&y, b, &formatdouble);

    result.sign = x.sign ^ y.sign;

    if (x.class == CLASS_NAN || y.class == CLASS_NAN || (x.class == CLASS_INFINITY && y.class == CLASS_INFINITY) || (x.class == CLASS_ZERO && y.class == CLASS_ZERO))
        return DOUBLE_NAN;

    if (x.class == CLASS_INFINITY || y.class == CLASS_ZERO || x.class == CLASS_ZERO || y.class == CLASS_INFINITY)
    {

        result.class = (x.class == CLASS_INFINITY || y.class == CLASS_ZERO) ? CLASS_INFINITY : CLASS_ZERO;

        return pack(&result, &formatdouble);

    }

    x.mantissa >>= 10;
    y.mantissa >>= 10;
    remainder = x.mantissa;
    result.mantissa = 0;

    for (i = 0; i < 63; i++)
    {

        result.mantissa <<= 1;

        if (remainder >= y.mantissa)
        {

            remainder -= y.mantissa;
            result.mantissa |= 1;

        }

        remainder <<= 1;

    }

    result.class = CLASS_NORMAL;
    result.exponent = x.exponent - y.exponent;
    result.mantissa |= (remainder) ? 1 : 0;

    return pack(&result, &formatdouble);

}

/* -1, 0 or 1, or 2 when either is a NaN */
static int compare(ulonglong a, ulonglong b)
{

    struct number x;
    struct number y;
    ulonglong magnitudea = a & ~((ulonglong)1 << 63);
    ulonglong magnitudeb = b & ~((ulonglong)1 << 63);

    unpack(&x, a, &formatdouble);
    unpack(&y, b, &formatdouble);

    if (x.class == CLASS_NAN || y.class == CLASS_NAN)
        return 2;

    if (!magnitudea && !magnitudeb)
        return 0;

    if (x.sign != y.sign)
        return (x.sign) ? -1 : 1;

    if (magnitudea == magnitudeb)
        return 0;

    return ((magnitudea < magnitudeb) ^ x.sign) ? -1 : 1;

}

static ulonglong fromunsigned(ulonglong value, unsigned int sign)
{

    struct number number;

    number.class = CLASS_NORMAL;
    number.sign = sign;
    number.exponent = 62;
    number.mantissa = value;

    if (!value)
        return 0;

    return pack(&number, &formatdouble);

}

/* the magnitude truncated toward zero, limit where it does not fit */
static ulonglong tounsigned(ulonglong a, ulonglong limit)
{

    struct number number;
    ulonglong value;

    unpack(&number, a, &formatdouble);

    if (number.class == CLASS_NAN || number.class == CLASS_ZERO || number.exponent < 0)
        return 0;

    if (number.class == CLASS_INFINITY || number.exponent > 63)
        return limit;

    value = (number.exponent == 63) ? number.mantissa << 1 : number.mantissa >> (62 - number.exponent);

    return (value > limit) ? limit : value;

}

static ulonglong widen(unsigned int a)
{

    struct number number;

    unpack(&number, a, &formatfloat);

    if (number.class == CLASS_NAN)
        return DOUBLE_NAN;

    return pack(&number, &formatdouble);

}

static unsigned int narrow(ulonglong a)
{

    struct number number;

    unpack(&number, a, &formatdouble);

    return pack(&number, &formatfloat);

}

ulonglong __aeabi_dadd(ulonglong a, ulonglong b)
{

    return add(a, b);

}

ulonglong __aeabi_dsub(ulonglong a, ulonglong b)
{

    return add(a, b ^ ((ulonglong)1 << 63));

}

ulonglong __aeabi_drsub(ulonglong a, ulonglong b)
{

    return add(b, a ^ ((ulonglong)1 << 63));

}

ulonglong __aeabi_dmul(ulonglong a, ulonglong b)
{

    return multiply(a, b);

}

ulonglong __aeabi_ddiv(ulonglong a, ulonglong b)
{

    return divide(a, b);

}

int __aeabi_dcmpeq(ulonglong a, ulonglong b)
{

    return compare(a, b) == 0;

}

int __aeabi_dcmplt(ulonglong a, ulonglong b)
{

    return compare(a, b) == -1;

}

int __aeabi_dcmple(ulonglong a, ulonglong b)
{

    int result = compare(a, b);

    return result == -1 || result == 0;

}

int __aeabi_dcmpge(ulonglong a, ulonglong b)
{

    int result = compare(a, b);

    return result == 1 || result == 0;

}

int __aeabi_dcmpgt(ulonglong a, ulonglong b)
{

    return compare(a, b) == 1;

}

int __aeabi_dcmpun(ulonglong a, ulonglong b)
{

    return compare(a, b) == 2;

}

ulonglong __aeabi_i2d(int a)
{

    return (a < 0) ? fromunsigned(-(unsigned int)a, 1) : fromunsigned(a, 0);

}

ulonglong __aeabi_ui2d(unsigned int a)
{

    return fromunsigned(a, 0);

}

ulonglong __aeabi_l2d(ulonglong a)
{

    return (a >> 63) ? fromunsigned(-a, 1) : fromunsigned(a, 0);

}

ulonglong __aeabi_ul2d(ulonglong a)
{

    return fromunsigned(a, 0);

}

int __aeabi_d2iz(ulonglong a)
{

    return (a >> 63) ? (int)(0 - (unsigned int)tounsigned(a, 0x80000000)) : (int)tounsigned(a, 0x7FFFFFFF);

}

unsigned int __aeabi_d2uiz(ulonglong a)
{

    return (a >> 63) ? 0 : tounsigned(a, 0xFFFFFFFF);

}

ulonglong __aeabi_d2lz(ulonglong a)
{

    return (a >> 63) ? -tounsigned(a, (ulonglong)1 << 63) : tounsigned(a, ((ulonglong)1 << 63) - 1);

}

ulonglong __aeabi_d2ulz(ulonglong a)
{

    return (a >> 63) ? 0 : tounsigned(a, ~(ulonglong)0);

}

ulonglong __aeabi_f2d(unsigned int a)
{

    return widen(a);

}

unsigned int __aeabi_d2f(ulonglong a)
{

    return narrow(a);

}

unsigned int __aeabi_fadd(unsigned int a, unsigned int b)
{

    return narrow(add(widen(a), widen(b)));

}

unsigned int __aeabi_fsub(unsigned int a, unsigned int b)
{

    return narrow(add(widen(a), widen(b ^ 0x80000000)));

}

unsigned int __aeabi_frsub(unsigned int a, unsigned int b)
{

    return narrow(add(widen(b), widen(a ^ 0x80000000)));

}

unsigned int __aeabi_fmul(unsigned int a, unsigned int b)
{

    return narrow(multiply(widen(a), widen(b)));

}

unsigned int __aeabi_fdiv(unsigned int a, unsigned int b)
{

    return narrow(divide(widen(a), widen(b)));

}

int __aeabi_fcmpeq(unsigned int a, unsigned int b)
{

    return compare(widen(a), widen(b)) == 0;

}

int __aeabi_fcmplt(unsigned int a, unsigned int b)
{

    return compare(widen(a), widen(b)) == -1;

}

int __aeabi_fcmple(unsigned int a, unsigned int b)
{

    int result = compare(widen(a), widen(b));

    return result == -1 || result == 0;

}

int __aeabi_fcmpge(unsigned int a, unsigned int b)
{

    int result = compare(widen(a), widen(b));

    return result == 1 || result == 0;

}

int __aeabi_fcmpgt(unsigned int a, unsigned int b)
{

    return compare(widen(a), widen(b)) == 1;

}

int __aeabi_fcmpun(unsigned int a, unsigned int b)
{

    return compare(widen(a), widen(b)) == 2;

}

unsigned int __aeabi_i2f(int a)
{

    return narrow(__aeabi_i2d(a));

}

unsigned int __aeabi_ui2f(unsigned int a)
{

    return narrow(fromunsigned(a, 0));

}

int __aeabi_f2iz(unsigned int a)
{

    return __aeabi_d2iz(widen(a));

}

unsigned int __aeabi_f2uiz(unsigned int a)
{

    return __aeabi_d2uiz(widen(a));

}

