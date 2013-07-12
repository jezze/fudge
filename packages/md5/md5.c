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

#define STEP(f, a, b, c, d, x, t, s) \
    (a) += f((b), (c), (d)) + (x) + (t); \
    (a) = (((a) << (s)) | ((a) >> (32 - (s)))); \
    (a) += (b);

#define SET(n) (*(unsigned int *)&ptr[(n) * 4])

static void *body(struct md5 *s, void *data, unsigned int size)
{

    unsigned char *ptr = data;
    unsigned int a, b, c, d;
    unsigned int saved_a, saved_b, saved_c, saved_d;

    a = s->a;
    b = s->b;
    c = s->c;
    d = s->d;

    do {

        saved_a = a;
        saved_b = b;
        saved_c = c;
        saved_d = d;

/* Round 1 */
        STEP(F, a, b, c, d, SET(0), 0xd76aa478, 7)
        STEP(F, d, a, b, c, SET(1), 0xe8c7b756, 12)
        STEP(F, c, d, a, b, SET(2), 0x242070db, 17)
        STEP(F, b, c, d, a, SET(3), 0xc1bdceee, 22)
        STEP(F, a, b, c, d, SET(4), 0xf57c0faf, 7)
        STEP(F, d, a, b, c, SET(5), 0x4787c62a, 12)
        STEP(F, c, d, a, b, SET(6), 0xa8304613, 17)
        STEP(F, b, c, d, a, SET(7), 0xfd469501, 22)
        STEP(F, a, b, c, d, SET(8), 0x698098d8, 7)
        STEP(F, d, a, b, c, SET(9), 0x8b44f7af, 12)
        STEP(F, c, d, a, b, SET(10), 0xffff5bb1, 17)
        STEP(F, b, c, d, a, SET(11), 0x895cd7be, 22)
        STEP(F, a, b, c, d, SET(12), 0x6b901122, 7)
        STEP(F, d, a, b, c, SET(13), 0xfd987193, 12)
        STEP(F, c, d, a, b, SET(14), 0xa679438e, 17)
        STEP(F, b, c, d, a, SET(15), 0x49b40821, 22)

/* Round 2 */
        STEP(G, a, b, c, d, SET(1), 0xf61e2562, 5)
        STEP(G, d, a, b, c, SET(6), 0xc040b340, 9)
        STEP(G, c, d, a, b, SET(11), 0x265e5a51, 14)
        STEP(G, b, c, d, a, SET(0), 0xe9b6c7aa, 20)
        STEP(G, a, b, c, d, SET(5), 0xd62f105d, 5)
        STEP(G, d, a, b, c, SET(10), 0x02441453, 9)
        STEP(G, c, d, a, b, SET(15), 0xd8a1e681, 14)
        STEP(G, b, c, d, a, SET(4), 0xe7d3fbc8, 20)
        STEP(G, a, b, c, d, SET(9), 0x21e1cde6, 5)
        STEP(G, d, a, b, c, SET(14), 0xc33707d6, 9)
        STEP(G, c, d, a, b, SET(3), 0xf4d50d87, 14)
        STEP(G, b, c, d, a, SET(8), 0x455a14ed, 20)
        STEP(G, a, b, c, d, SET(13), 0xa9e3e905, 5)
        STEP(G, d, a, b, c, SET(2), 0xfcefa3f8, 9)
        STEP(G, c, d, a, b, SET(7), 0x676f02d9, 14)
        STEP(G, b, c, d, a, SET(12), 0x8d2a4c8a, 20)

/* Round 3 */
        STEP(H, a, b, c, d, SET(5), 0xfffa3942, 4)
        STEP(H, d, a, b, c, SET(8), 0x8771f681, 11)
        STEP(H, c, d, a, b, SET(11), 0x6d9d6122, 16)
        STEP(H, b, c, d, a, SET(14), 0xfde5380c, 23)
        STEP(H, a, b, c, d, SET(1), 0xa4beea44, 4)
        STEP(H, d, a, b, c, SET(4), 0x4bdecfa9, 11)
        STEP(H, c, d, a, b, SET(7), 0xf6bb4b60, 16)
        STEP(H, b, c, d, a, SET(10), 0xbebfbc70, 23)
        STEP(H, a, b, c, d, SET(13), 0x289b7ec6, 4)
        STEP(H, d, a, b, c, SET(0), 0xeaa127fa, 11)
        STEP(H, c, d, a, b, SET(3), 0xd4ef3085, 16)
        STEP(H, b, c, d, a, SET(6), 0x04881d05, 23)
        STEP(H, a, b, c, d, SET(9), 0xd9d4d039, 4)
        STEP(H, d, a, b, c, SET(12), 0xe6db99e5, 11)
        STEP(H, c, d, a, b, SET(15), 0x1fa27cf8, 16)
        STEP(H, b, c, d, a, SET(2), 0xc4ac5665, 23)

/* Round 4 */
        STEP(I, a, b, c, d, SET(0), 0xf4292244, 6)
        STEP(I, d, a, b, c, SET(7), 0x432aff97, 10)
        STEP(I, c, d, a, b, SET(14), 0xab9423a7, 15)
        STEP(I, b, c, d, a, SET(5), 0xfc93a039, 21)
        STEP(I, a, b, c, d, SET(12), 0x655b59c3, 6)
        STEP(I, d, a, b, c, SET(3), 0x8f0ccc92, 10)
        STEP(I, c, d, a, b, SET(10), 0xffeff47d, 15)
        STEP(I, b, c, d, a, SET(1), 0x85845dd1, 21)
        STEP(I, a, b, c, d, SET(8), 0x6fa87e4f, 6)
        STEP(I, d, a, b, c, SET(15), 0xfe2ce6e0, 10)
        STEP(I, c, d, a, b, SET(6), 0xa3014314, 15)
        STEP(I, b, c, d, a, SET(13), 0x4e0811a1, 21)
        STEP(I, a, b, c, d, SET(4), 0xf7537e82, 6)
        STEP(I, d, a, b, c, SET(11), 0xbd3af235, 10)
        STEP(I, c, d, a, b, SET(2), 0x2ad7d2bb, 15)
        STEP(I, b, c, d, a, SET(9), 0xeb86d391, 21)

        a += saved_a;
        b += saved_b;
        c += saved_c;
        d += saved_d;
        ptr += 64;

    } while (size -= 64);

    s->a = a;
    s->b = b;
    s->c = c;
    s->d = d;

    return ptr;

}

void md5_init(struct md5 *s)
{

    s->a = 0x67452301;
    s->b = 0xefcdab89;
    s->c = 0x98badcfe;
    s->d = 0x10325476;
    s->lo = 0;
    s->hi = 0;

}

void md5_update(struct md5 *s, void *data, unsigned int size)
{

    unsigned int saved_lo;
    unsigned int used, free;

    saved_lo = s->lo;

    if ((s->lo = (saved_lo + size) & 0x1fffffff) < saved_lo)
        s->hi++;

    s->hi += size >> 29;
    used = saved_lo & 0x3f;

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

        data = body(s, data, size & ~(unsigned int)0x3f);
        size &= 0x3f;

    }

    memory_copy(s->buffer, data, size);

}

void md5_final(struct md5 *s, unsigned char *digest)
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

