#include <fudge.h>
#include "md5.h"

#define F(x, y, z)                      (z ^ (x & (y ^ z)))
#define G(x, y, z)                      (y ^ (z & (y ^ x)))
#define H(x, y, z)                      (x ^ y ^ z)
#define I(x, y, z)                      (y ^ (x | ~z))
#define FF(a, b, c, d, w, s, t)         a += F(b, c, d) + w + t; a = rol(a, s) + b
#define GG(a, b, c, d, w, s, t)         a += G(b, c, d) + w + t; a = rol(a, s) + b
#define HH(a, b, c, d, w, s, t)         a += H(b, c, d) + w + t; a = rol(a, s) + b
#define II(a, b, c, d, w, s, t)         a += I(b, c, d) + w + t; a = rol(a, s) + b

static unsigned int tab[64] = {
    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

static unsigned int rol(unsigned int n, int k)
{

    return (n << k) | (n >> (32 - k));

}

static void processblock(struct md5 *s, unsigned char *buffer)
{

    unsigned int W[16];
    unsigned int a, b, c, d;
    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        W[i] = buffer[4 * i];
        W[i] |= (unsigned int)buffer[4 * i + 1] << 8;
        W[i] |= (unsigned int)buffer[4 * i + 2] << 16;
        W[i] |= (unsigned int)buffer[4 * i + 3] << 24;

    }

    a = s->a;
    b = s->b;
    c = s->c;
    d = s->d;
    i = 0;

    while (i < 16)
    {

        FF(a, b, c, d, W[i],  7, tab[i]); i++;
        FF(d, a, b, c, W[i], 12, tab[i]); i++;
        FF(c, d, a, b, W[i], 17, tab[i]); i++;
        FF(b, c, d, a, W[i], 22, tab[i]); i++;

    }

    while (i < 32)
    {

        GG(a, b, c, d, W[(5 * i + 1) % 16],  5, tab[i]); i++;
        GG(d, a, b, c, W[(5 * i + 1) % 16],  9, tab[i]); i++;
        GG(c, d, a, b, W[(5 * i + 1) % 16], 14, tab[i]); i++;
        GG(b, c, d, a, W[(5 * i + 1) % 16], 20, tab[i]); i++;

    }

    while (i < 48)
    {

        HH(a, b, c, d, W[(3 * i + 5) % 16],  4, tab[i]); i++;
        HH(d, a, b, c, W[(3 * i + 5) % 16], 11, tab[i]); i++;
        HH(c, d, a, b, W[(3 * i + 5) % 16], 16, tab[i]); i++;
        HH(b, c, d, a, W[(3 * i + 5) % 16], 23, tab[i]); i++;

    }

    while (i < 64)
    {

        II(a, b, c, d, W[7 * i % 16],  6, tab[i]); i++;
        II(d, a, b, c, W[7 * i % 16], 10, tab[i]); i++;
        II(c, d, a, b, W[7 * i % 16], 15, tab[i]); i++;
        II(b, c, d, a, W[7 * i % 16], 21, tab[i]); i++;

    }

    s->a += a;
    s->b += b;
    s->c += c;
    s->d += d;

}

void md5_read(struct md5 *s, void *buffer, unsigned int count)
{

    unsigned int r = s->lo & 0x3F;
    unsigned char *p = buffer;
    unsigned int temp = s->lo;

    s->lo = (s->lo + count) & 0x1FFFFFFF;
    s->hi += count >> 29;

    if (s->lo < temp)
        s->hi++;

    if (r)
    {

        unsigned int c = 64 - r;

        if (count < c)
        {

            buffer_copy(s->buffer + r, p, count);

            return;

        }

        buffer_copy(s->buffer + r, p, c);
        processblock(s, s->buffer);

        p += c;
        count -= c;

    }

    for (; count >= 64; count -= 64, p += 64)
        processblock(s, p);

    buffer_copy(s->buffer, p, count);

}

void md5_write(struct md5 *s, unsigned char digest[16])
{

    unsigned r = s->lo & 0x3F;

    s->buffer[r++] = 0x80;

    if (r > 56)
    {

        buffer_clear(s->buffer + r, 64 - r);
        processblock(s, s->buffer);

        r = 0;

    }

    buffer_clear(s->buffer + r, 56 - r);

    s->lo <<= 3;
    s->buffer[56] = s->lo;
    s->buffer[57] = s->lo >> 8;
    s->buffer[58] = s->lo >> 16;
    s->buffer[59] = s->lo >> 24;
    s->buffer[60] = s->hi;
    s->buffer[61] = s->hi >> 8;
    s->buffer[62] = s->hi >> 16;
    s->buffer[63] = s->hi >> 24;

    processblock(s, s->buffer);

    digest[0] = s->a;
    digest[1] = s->a >> 8;
    digest[2] = s->a >> 16;
    digest[3] = s->a >> 24;
    digest[4] = s->b;
    digest[5] = s->b >> 8;
    digest[6] = s->b >> 16;
    digest[7] = s->b >> 24;
    digest[8] = s->c;
    digest[9] = s->c >> 8;
    digest[10] = s->c >> 16;
    digest[11] = s->c >> 24;
    digest[12] = s->d;
    digest[13] = s->d >> 8;
    digest[14] = s->d >> 16;
    digest[15] = s->d >> 24;

}

void md5_init(struct md5 *s)
{

    s->lo = 0;
    s->hi = 0;
    s->a = 0x67452301;
    s->b = 0xEFCDAB89;
    s->c = 0x98BADCFE;
    s->d = 0x10325476;

}

