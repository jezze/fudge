#include <fudge.h>

struct sha1
{

	unsigned int len;
	unsigned int hi;
	unsigned int h[5];
	unsigned char buf[64];

};

#define F0(b,c,d) (d ^ (b & (c ^ d)))
#define F1(b,c,d) (b ^ c ^ d)
#define F2(b,c,d) ((b & c) | (d & (b | c)))
#define F3(b,c,d) (b ^ c ^ d)
#define G0(a,b,c,d,e,i) e += rol(a,5) + F0(b, c, d) + W[i] + 0x5A827999; b = rol(b, 30)
#define G1(a,b,c,d,e,i) e += rol(a,5) + F1(b, c, d) + W[i] + 0x6ED9EBA1; b = rol(b, 30)
#define G2(a,b,c,d,e,i) e += rol(a,5) + F2(b, c, d) + W[i] + 0x8F1BBCDC; b = rol(b, 30)
#define G3(a,b,c,d,e,i) e += rol(a,5) + F3(b, c, d) + W[i] + 0xCA62C1D6; b = rol(b, 30)

static unsigned int rol(unsigned int n, int k)
{

    return (n << k) | (n >> (32 - k));

}

static void processblock(struct sha1 *s, const unsigned char *buf)
{

	unsigned int W[80], a, b, c, d, e;
	int i;

	for (i = 0; i < 16; i++)
    {

		W[i] = (unsigned int)buf[4 * i] << 24;
		W[i] |= (unsigned int)buf[4 * i + 1] << 16;
		W[i] |= (unsigned int)buf[4 * i + 2] << 8;
		W[i] |= buf[4 * i + 3];

	}

	for (; i < 80; i++)
		W[i] = rol(W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16], 1);

	a = s->h[0];
	b = s->h[1];
	c = s->h[2];
	d = s->h[3];
	e = s->h[4];

	for (i = 0; i < 20;)
    {

		G0(a,b,c,d,e,i++);
		G0(e,a,b,c,d,i++);
		G0(d,e,a,b,c,i++);
		G0(c,d,e,a,b,i++);
		G0(b,c,d,e,a,i++);
	}

	while (i < 40)
    {

		G1(a,b,c,d,e,i++);
		G1(e,a,b,c,d,i++);
		G1(d,e,a,b,c,i++);
		G1(c,d,e,a,b,i++);
		G1(b,c,d,e,a,i++);

	}

	while (i < 60)
    {

		G2(a,b,c,d,e,i++);
		G2(e,a,b,c,d,i++);
		G2(d,e,a,b,c,i++);
		G2(c,d,e,a,b,i++);
		G2(b,c,d,e,a,i++);

	}

	while (i < 80)
    {

		G3(a,b,c,d,e,i++);
		G3(e,a,b,c,d,i++);
		G3(d,e,a,b,c,i++);
		G3(c,d,e,a,b,i++);
		G3(b,c,d,e,a,i++);

	}

	s->h[0] += a;
	s->h[1] += b;
	s->h[2] += c;
	s->h[3] += d;
	s->h[4] += e;

}

static void pad(struct sha1 *s)
{

	unsigned r = s->len % 64;

	s->buf[r++] = 0x80;

	if (r > 56)
    {

		memory_clear(s->buf + r, 64 - r);

		r = 0;

		processblock(s, s->buf);

	}

	memory_clear(s->buf + r, 56 - r);

	s->len *= 8;
	s->buf[56] = s->hi >> 24;
	s->buf[57] = s->hi >> 16;
	s->buf[58] = s->hi >> 8;
	s->buf[59] = s->hi;
	s->buf[60] = s->len >> 24;
	s->buf[61] = s->len >> 16;
	s->buf[62] = s->len >> 8;
	s->buf[63] = s->len;

	processblock(s, s->buf);

}

static void sha1_read(struct sha1 *s, unsigned int len, const void *m)
{

	const unsigned char *p = m;
	unsigned r = s->len % 64;

	s->len += len;

	if (r)
    {

		if (len < 64 - r)
        {

			memory_copy(s->buf + r, p, len);

			return;

		}

		memory_copy(s->buf + r, p, 64 - r);

		len -= 64 - r;
		p += 64 - r;

		processblock(s, s->buf);
        
	}

	for (; len >= 64; len -= 64, p += 64)
		processblock(s, p);

	memory_copy(s->buf, p, len);

}

static void sha1_write(struct sha1 *s, unsigned int count, unsigned char md[20])
{

	int i;

	pad(s);

	for (i = 0; i < 5; i++)
    {

		md[4*i] = s->h[i] >> 24;
		md[4*i+1] = s->h[i] >> 16;
		md[4*i+2] = s->h[i] >> 8;
		md[4*i+3] = s->h[i];

	}

}

static void sha1_init(struct sha1 *s)
{

    memory_clear(s, sizeof (struct sha1));

	s->h[0] = 0x67452301;
	s->h[1] = 0xEFCDAB89;
	s->h[2] = 0x98BADCFE;
	s->h[3] = 0x10325476;
	s->h[4] = 0xC3D2E1F0;

}

void main()
{

    struct sha1 s;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned char digest[20];
    unsigned char num[32];
    unsigned int count;
    unsigned int offset;
    unsigned int i;

    sha1_init(&s);

    for (offset = 0; (count = call_read(CALL_DI, offset, FUDGE_BSIZE, buffer)); offset += count)
        sha1_read(&s, count, buffer);

    sha1_write(&s, 20, digest);

    for (i = 0; i < 20; i++)
        call_write(CALL_DO, 0, memory_write_paddednumber(num, 32, digest[i], 16, 2, 0), num);

}

