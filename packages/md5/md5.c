#include <fudge.h>

struct md5
{

    unsigned int lo, hi;
    unsigned int a, b, c, d;
    unsigned char buffer[64];

};

static unsigned int rol(unsigned int n, int k) { return (n << k) | (n >> (32-k)); }
#define F(x,y,z) (z ^ (x & (y ^ z)))
#define G(x,y,z) (y ^ (z & (y ^ x)))
#define H(x,y,z) (x ^ y ^ z)
#define I(x,y,z) (y ^ (x | ~z))
#define FF(a,b,c,d,w,s,t) a += F(b,c,d) + w + t; a = rol(a,s) + b
#define GG(a,b,c,d,w,s,t) a += G(b,c,d) + w + t; a = rol(a,s) + b
#define HH(a,b,c,d,w,s,t) a += H(b,c,d) + w + t; a = rol(a,s) + b
#define II(a,b,c,d,w,s,t) a += I(b,c,d) + w + t; a = rol(a,s) + b

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

static void processblock(struct md5 *s, const unsigned char *buffer)
{

	unsigned int i, W[16], a, b, c, d;

	for (i = 0; i < 16; i++) {
		W[i] = buffer[4*i];
		W[i] |= (unsigned int)buffer[4*i+1]<<8;
		W[i] |= (unsigned int)buffer[4*i+2]<<16;
		W[i] |= (unsigned int)buffer[4*i+3]<<24;
	}

	a = s->a;
	b = s->b;
	c = s->c;
	d = s->d;

	i = 0;
	while (i < 16) {
		FF(a,b,c,d, W[i],  7, tab[i]); i++;
		FF(d,a,b,c, W[i], 12, tab[i]); i++;
		FF(c,d,a,b, W[i], 17, tab[i]); i++;
		FF(b,c,d,a, W[i], 22, tab[i]); i++;
	}
	while (i < 32) {
		GG(a,b,c,d, W[(5*i+1)%16],  5, tab[i]); i++;
		GG(d,a,b,c, W[(5*i+1)%16],  9, tab[i]); i++;
		GG(c,d,a,b, W[(5*i+1)%16], 14, tab[i]); i++;
		GG(b,c,d,a, W[(5*i+1)%16], 20, tab[i]); i++;
	}
	while (i < 48) {
		HH(a,b,c,d, W[(3*i+5)%16],  4, tab[i]); i++;
		HH(d,a,b,c, W[(3*i+5)%16], 11, tab[i]); i++;
		HH(c,d,a,b, W[(3*i+5)%16], 16, tab[i]); i++;
		HH(b,c,d,a, W[(3*i+5)%16], 23, tab[i]); i++;
	}
	while (i < 64) {
		II(a,b,c,d, W[7*i%16],  6, tab[i]); i++;
		II(d,a,b,c, W[7*i%16], 10, tab[i]); i++;
		II(c,d,a,b, W[7*i%16], 15, tab[i]); i++;
		II(b,c,d,a, W[7*i%16], 21, tab[i]); i++;
	}

	s->a += a;
	s->b += b;
	s->c += c;
	s->d += d;

}

static void pad(struct md5 *s)
{

    unsigned r = s->lo % 64;

    s->buffer[r++] = 0x80;

    if (r > 56)
    {

        memory_clear(s->buffer + r, 64 - r);

        r = 0;

        processblock(s, s->buffer);

    }

    memory_clear(s->buffer + r, 56 - r);

    s->lo *= 8;
    s->buffer[56] = s->lo;
    s->buffer[57] = s->lo >> 8;
    s->buffer[58] = s->lo >> 16;
    s->buffer[59] = s->lo >> 24;
    s->buffer[60] = s->hi;
    s->buffer[61] = s->hi >> 8;
    s->buffer[62] = s->hi >> 16;
    s->buffer[63] = s->hi >> 24;

    processblock(s, s->buffer);

}

static void md5_read(struct md5 *s, unsigned int count, void *buffer)
{

    unsigned char *p = buffer;
    unsigned int lo = s->lo;
    unsigned int r = s->lo & 0x3f;

    if ((s->lo = (lo + count) & 0x1fffffff) < lo)
        s->hi++;

    s->hi += count >> 29;

    if (r)
    {

        if (count < 64 - r)
        {

            memory_copy(s->buffer + r, p, count);

            return;

        }

        memory_copy(s->buffer + r, p, 64 - r);

        p += 64 - r;
        count -= 64 - r;

        processblock(s, s->buffer);

    }

    for (; count >= 64; count -= 64, p += 64)
        processblock(s, p);

    memory_copy(s->buffer, p, count);

}

static void md5_write(struct md5 *s, unsigned char digest[16])
{

    pad(s);

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
        md5_read(&s, count, buffer);

    md5_write(&s, digest);

    for (i = 0; i < 16; i++)
        call_write(CALL_DO, 0, memory_write_paddednumber(num, 32, digest[i], 16, 2, 0), num);

}

