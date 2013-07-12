#include <fudge.h>

struct md5
{

    unsigned int lo, hi;
    unsigned int a, b, c, d;
    unsigned char buffer[64];
    unsigned int block[16];

};

#define F(x, y, z)                      ((z) ^ ((x) & ((y) ^ (z))))
#define G(x, y, z)                      ((y) ^ ((z) & ((x) ^ (y))))
#define H(x, y, z)                      ((x) ^ (y) ^ (z))
#define I(x, y, z)                      ((y) ^ ((x) | ~(z)))

#define STEP(f, a, b, c, d, t, x, s) \
    (a) += f((b), (c), (d)) + (x) + (tab[t]); \
    (a) = (((a) << (s)) | ((a) >> (32 - (s)))); \
    (a) += (b);

#define SET(n) (*(unsigned int *)(current + (n * 4)))

static const unsigned int tab[64] =
{

    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391

};

static void *body(struct md5 *s, void *data, unsigned int size)
{

    unsigned char *current = data;
    unsigned int a, b, c, d;
    unsigned int sa, sb, sc, sd;

    a = s->a;
    b = s->b;
    c = s->c;
    d = s->d;

    do {

        sa = a;
        sb = b;
        sc = c;
        sd = d;

        STEP(F, a, b, c, d, 0x00, SET(0), 7)
        STEP(F, d, a, b, c, 0x01, SET(1), 12)
        STEP(F, c, d, a, b, 0x02, SET(2), 17)
        STEP(F, b, c, d, a, 0x03, SET(3), 22)
        STEP(F, a, b, c, d, 0x04, SET(4), 7)
        STEP(F, d, a, b, c, 0x05, SET(5), 12)
        STEP(F, c, d, a, b, 0x06, SET(6), 17)
        STEP(F, b, c, d, a, 0x07, SET(7), 22)
        STEP(F, a, b, c, d, 0x08, SET(8), 7)
        STEP(F, d, a, b, c, 0x09, SET(9), 12)
        STEP(F, c, d, a, b, 0x0A, SET(10), 17)
        STEP(F, b, c, d, a, 0x0B, SET(11), 22)
        STEP(F, a, b, c, d, 0x0C, SET(12), 7)
        STEP(F, d, a, b, c, 0x0D, SET(13), 12)
        STEP(F, c, d, a, b, 0x0E, SET(14), 17)
        STEP(F, b, c, d, a, 0x0F, SET(15), 22)
        STEP(G, a, b, c, d, 0x10, SET(1), 5)
        STEP(G, d, a, b, c, 0x11, SET(6), 9)
        STEP(G, c, d, a, b, 0x12, SET(11), 14)
        STEP(G, b, c, d, a, 0x13, SET(0), 20)
        STEP(G, a, b, c, d, 0x14, SET(5), 5)
        STEP(G, d, a, b, c, 0x15, SET(10), 9)
        STEP(G, c, d, a, b, 0x16, SET(15), 14)
        STEP(G, b, c, d, a, 0x17, SET(4), 20)
        STEP(G, a, b, c, d, 0x18, SET(9), 5)
        STEP(G, d, a, b, c, 0x19, SET(14), 9)
        STEP(G, c, d, a, b, 0x1A, SET(3), 14)
        STEP(G, b, c, d, a, 0x1B, SET(8), 20)
        STEP(G, a, b, c, d, 0x1C, SET(13), 5)
        STEP(G, d, a, b, c, 0x1D, SET(2), 9)
        STEP(G, c, d, a, b, 0x1E, SET(7), 14)
        STEP(G, b, c, d, a, 0x1F, SET(12), 20)
        STEP(H, a, b, c, d, 0x20, SET(5), 4)
        STEP(H, d, a, b, c, 0x21, SET(8), 11)
        STEP(H, c, d, a, b, 0x22, SET(11), 16)
        STEP(H, b, c, d, a, 0x23, SET(14), 23)
        STEP(H, a, b, c, d, 0x24, SET(1), 4)
        STEP(H, d, a, b, c, 0x25, SET(4), 11)
        STEP(H, c, d, a, b, 0x26, SET(7), 16)
        STEP(H, b, c, d, a, 0x27, SET(10), 23)
        STEP(H, a, b, c, d, 0x28, SET(13), 4)
        STEP(H, d, a, b, c, 0x29, SET(0), 11)
        STEP(H, c, d, a, b, 0x2A, SET(3), 16)
        STEP(H, b, c, d, a, 0x2B, SET(6), 23)
        STEP(H, a, b, c, d, 0x2C, SET(9), 4)
        STEP(H, d, a, b, c, 0x2D, SET(12), 11)
        STEP(H, c, d, a, b, 0x2E, SET(15), 16)
        STEP(H, b, c, d, a, 0x2F, SET(2), 23)
        STEP(I, a, b, c, d, 0x30, SET(0), 6)
        STEP(I, d, a, b, c, 0x31, SET(7), 10)
        STEP(I, c, d, a, b, 0x32, SET(14), 15)
        STEP(I, b, c, d, a, 0x33, SET(5), 21)
        STEP(I, a, b, c, d, 0x34, SET(12), 6)
        STEP(I, d, a, b, c, 0x35, SET(3), 10)
        STEP(I, c, d, a, b, 0x36, SET(10), 15)
        STEP(I, b, c, d, a, 0x37, SET(1), 21)
        STEP(I, a, b, c, d, 0x38, SET(8), 6)
        STEP(I, d, a, b, c, 0x39, SET(15), 10)
        STEP(I, c, d, a, b, 0x3A, SET(6), 15)
        STEP(I, b, c, d, a, 0x3B, SET(13), 21)
        STEP(I, a, b, c, d, 0x3C, SET(4), 6)
        STEP(I, d, a, b, c, 0x3D, SET(11), 10)
        STEP(I, c, d, a, b, 0x3E, SET(2), 15)
        STEP(I, b, c, d, a, 0x3F, SET(9), 21)

        a += sa;
        b += sb;
        c += sc;
        d += sd;
        current += 64;

    } while (size -= 64);

    s->a = a;
    s->b = b;
    s->c = c;
    s->d = d;

    return current;

}

static void md5_update(struct md5 *s, void *data, unsigned int size)
{

    unsigned int lo;
    unsigned int used, free;

    lo = s->lo;

    if ((s->lo = (lo + size) & 0x1fffffff) < lo)
        s->hi++;

    s->hi += size >> 29;
    used = lo & 0x3f;

    if (used)
    {

        free = 64 - used;

        if (size < free)
        {

            memory_copy(&s->buffer[used], data, size);

            return;

        }

        memory_copy(&s->buffer[used], data, free);
        data = (unsigned char *)data + free;
        size -= free;
        body(s, s->buffer, 64);

    }

    if (size >= 64)
    {

        data = body(s, data, size & ~0x3f);
        size &= 0x3f;

    }

    memory_copy(s->buffer, data, size);

}

static void md5_final(struct md5 *s, unsigned char *digest)
{

    unsigned int used, free;

    used = s->lo & 0x3f;
    s->buffer[used++] = 0x80;
    free = 64 - used;

    if (free < 8)
    {

        memory_clear(&s->buffer[used], free);
        body(s, s->buffer, 64);

        used = 0;
        free = 64;

    }

    memory_clear(&s->buffer[used], free - 8);

    s->lo <<= 3;
    s->buffer[56] = s->lo;
    s->buffer[57] = s->lo >> 8;
    s->buffer[58] = s->lo >> 16;
    s->buffer[59] = s->lo >> 24;
    s->buffer[60] = s->hi;
    s->buffer[61] = s->hi >> 8;
    s->buffer[62] = s->hi >> 16;
    s->buffer[63] = s->hi >> 24;

    body(s, s->buffer, 64);

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

static void md5_init(struct md5 *s)
{

    memory_clear(s, sizeof (struct md5));

    s->a = 0x67452301;
    s->b = 0xefcdab89;
    s->c = 0x98badcfe;
    s->d = 0x10325476;

}

void main()
{

    struct md5 s;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned char digest[16];
    unsigned char num[32];
    unsigned int count;
    unsigned int offset;
    unsigned int i;

    md5_init(&s);

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += count)
        md5_update(&s, buffer, count);

    md5_final(&s, digest);

    for (i = 0; i < 16; i++)
        call_write(CALL_DO, 0, memory_write_paddednumber(num, 32, digest[i], 16, 2, 0), num);

}

