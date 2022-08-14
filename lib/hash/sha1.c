#include <fudge.h>
#include "sha1.h"

#define F0(b, c, d)                     (d ^ (b & (c ^ d)))
#define F1(b, c, d)                     (b ^ c ^ d)
#define F2(b, c, d)                     ((b & c) | (d & (b | c)))
#define F3(b, c, d)                     (b ^ c ^ d)
#define G0(a, b, c, d, e, w)            e += rol(a, 5) + F0(b, c, d) + w + 0x5A827999; b = rol(b, 30)
#define G1(a, b, c, d, e, w)            e += rol(a, 5) + F1(b, c, d) + w + 0x6ED9EBA1; b = rol(b, 30)
#define G2(a, b, c, d, e, w)            e += rol(a, 5) + F2(b, c, d) + w + 0x8F1BBCDC; b = rol(b, 30)
#define G3(a, b, c, d, e, w)            e += rol(a, 5) + F3(b, c, d) + w + 0xCA62C1D6; b = rol(b, 30)

static unsigned int rol(unsigned int n, int k)
{

    return (n << k) | (n >> (32 - k));

}

static void processblock(struct sha1 *s, unsigned char *buffer)
{

    unsigned int W[80];
    unsigned int a, b, c, d, e;
    unsigned int i;

    for (i = 0; i < 16; i++)
    {

        W[i] = (unsigned int)buffer[4 * i] << 24;
        W[i] |= (unsigned int)buffer[4 * i + 1] << 16;
        W[i] |= (unsigned int)buffer[4 * i + 2] << 8;
        W[i] |= buffer[4 * i + 3];

    }

    for (; i < 80; i++)
        W[i] = rol(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);

    a = s->a;
    b = s->b;
    c = s->c;
    d = s->d;
    e = s->e;
    i = 0;

    while (i < 20)
    {

        G0(a, b, c, d, e, W[i]); i++;
        G0(e, a, b, c, d, W[i]); i++;
        G0(d, e, a, b, c, W[i]); i++;
        G0(c, d, e, a, b, W[i]); i++;
        G0(b, c, d, e, a, W[i]); i++;

    }

    while (i < 40)
    {

        G1(a, b, c, d, e, W[i]); i++;
        G1(e, a, b, c, d, W[i]); i++;
        G1(d, e, a, b, c, W[i]); i++;
        G1(c, d, e, a, b, W[i]); i++;
        G1(b, c, d, e, a, W[i]); i++;

    }

    while (i < 60)
    {

        G2(a, b, c, d, e, W[i]); i++;
        G2(e, a, b, c, d, W[i]); i++;
        G2(d, e, a, b, c, W[i]); i++;
        G2(c, d, e, a, b, W[i]); i++;
        G2(b, c, d, e, a, W[i]); i++;

    }

    while (i < 80)
    {

        G3(a, b, c, d, e, W[i]); i++;
        G3(e, a, b, c, d, W[i]); i++;
        G3(d, e, a, b, c, W[i]); i++;
        G3(c, d, e, a, b, W[i]); i++;
        G3(b, c, d, e, a, W[i]); i++;

    }

    s->a += a;
    s->b += b;
    s->c += c;
    s->d += d;
    s->e += e;

}

void sha1_read(struct sha1 *s, void *buffer, unsigned int count)
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

void sha1_write(struct sha1 *s, unsigned char digest[20])
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
    s->buffer[56] = s->hi >> 24;
    s->buffer[57] = s->hi >> 16;
    s->buffer[58] = s->hi >> 8;
    s->buffer[59] = s->hi;
    s->buffer[60] = s->lo >> 24;
    s->buffer[61] = s->lo >> 16;
    s->buffer[62] = s->lo >> 8;
    s->buffer[63] = s->lo;

    processblock(s, s->buffer);

    digest[0] = s->a >> 24;
    digest[1] = s->a >> 16;
    digest[2] = s->a >> 8;
    digest[3] = s->a;
    digest[4] = s->b >> 24;
    digest[5] = s->b >> 16;
    digest[6] = s->b >> 8;
    digest[7] = s->b;
    digest[8] = s->c >> 24;
    digest[9] = s->c >> 16;
    digest[10] = s->c >> 8;
    digest[11] = s->c;
    digest[12] = s->d >> 24;
    digest[13] = s->d >> 16;
    digest[14] = s->d >> 8;
    digest[15] = s->d;
    digest[16] = s->e >> 24;
    digest[17] = s->e >> 16;
    digest[18] = s->e >> 8;
    digest[19] = s->e;

}

void sha1_init(struct sha1 *s)
{

    s->lo = 0;
    s->hi = 0;
    s->a = 0x67452301;
    s->b = 0xEFCDAB89;
    s->c = 0x98BADCFE;
    s->d = 0x10325476;
    s->e = 0xC3D2E1F0;

}

