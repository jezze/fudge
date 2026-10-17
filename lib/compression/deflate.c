#include "deflate.h"

#define MAXBITS                         15
#define COUNTS                          DEFLATE_COUNTS
#define LITERALS                        DEFLATE_LITERALS
#define FASTBITS                        DEFLATE_FASTBITS
#define LENGTHS                         29
#define DISTANCES                       30
#define CODELENGTHS                     19
#define BLOCK_NONE                      0
#define BLOCK_STORED                    1
#define BLOCK_CODES                     2

static unsigned short lengthbase[LENGTHS] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static unsigned char lengthextra[LENGTHS] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static unsigned short distancebase[DISTANCES] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
static unsigned char distanceextra[DISTANCES] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
static unsigned char codelengthorder[CODELENGTHS] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

static void putbyte(struct deflate *d, unsigned char value)
{

    if (d->ring)
        d->out[d->ooffset++ & (d->ocount - 1)] = value;
    else if (d->ooffset == d->ocount)
        d->status = DEFLATE_FULL;
    else
        d->out[d->ooffset++] = value;

}

static unsigned char getwindow(struct deflate *d, unsigned int distance)
{

    unsigned int offset = d->ooffset - distance;

    return d->out[(d->ring) ? offset & (d->ocount - 1) : offset];

}

static unsigned int getbits(struct deflate *d, unsigned int count)
{

    unsigned int value = d->bits;

    while (d->nbits < count)
    {

        if (d->ioffset == d->icount && !(d->refill && d->refill(d)))
        {

            d->status = DEFLATE_INVALID;

            return 0;

        }

        value |= (unsigned int)d->in[d->ioffset++] << d->nbits;
        d->nbits += 8;

    }

    d->bits = value >> count;
    d->nbits -= count;

    return value & ((1 << count) - 1);

}

/* reads ahead up to count bits while there is input, without failing */
static void fill(struct deflate *d, unsigned int count)
{

    while (d->nbits < count && (d->ioffset < d->icount || (d->refill && d->refill(d))))
    {

        d->bits |= (unsigned int)d->in[d->ioffset++] << d->nbits;
        d->nbits += 8;

    }

}

/* whole bytes read ahead go back to the input */
static void giveback(struct deflate *d)
{

    while (d->nbits >= 8 && d->ioffset)
    {

        d->ioffset--;
        d->nbits -= 8;

    }

    d->bits &= (1 << d->nbits) - 1;

}

/* an entry holds the code length above the 9 bits of the symbol, at every index whose low bits are the code, reversed as the bits arrive */
static void fast(struct deflate_huffman *h)
{

    unsigned int code = 0;
    unsigned int index = 0;
    unsigned int length;
    unsigned int i;

    for (i = 0; i < (1 << FASTBITS); i++)
        h->fast[i] = 0;

    for (length = 1; length <= FASTBITS; length++)
    {

        for (i = 0; i < h->count[length]; i++)
        {

            unsigned int reversed = 0;
            unsigned int j;

            for (j = 0; j < length; j++)
                reversed |= ((code >> j) & 1) << (length - 1 - j);

            for (j = reversed; j < (1 << FASTBITS); j += 1 << length)
                h->fast[j] = (length << 9) | h->symbol[index + i];

            code++;

        }

        index += h->count[length];
        code <<= 1;

    }

}

static unsigned int build(struct deflate_huffman *h, unsigned short *lengths, unsigned int n)
{

    unsigned short offsets[COUNTS];
    unsigned int left = 1;
    unsigned int i;

    for (i = 0; i < COUNTS; i++)
        h->count[i] = 0;

    for (i = 0; i < n; i++)
        h->count[lengths[i]]++;

    for (i = 1; i < COUNTS; i++)
    {

        left <<= 1;

        if (h->count[i] > left)
            return 0;

        left -= h->count[i];

    }

    offsets[1] = 0;

    for (i = 1; i < MAXBITS; i++)
        offsets[i + 1] = offsets[i] + h->count[i];

    for (i = 0; i < n; i++)
    {

        if (lengths[i])
            h->symbol[offsets[lengths[i]]++] = i;

    }

    fast(h);

    return 1;

}

/* codes up to FASTBITS long decode with one lookup, the rest a bit at a time */
static unsigned int decode(struct deflate *d, struct deflate_huffman *h)
{

    unsigned int entry;
    unsigned int code = 0;
    unsigned int first = 0;
    unsigned int index = 0;
    unsigned int i;

    fill(d, FASTBITS);

    entry = h->fast[d->bits & ((1 << FASTBITS) - 1)];

    if (entry && (entry >> 9) <= d->nbits)
    {

        d->bits >>= entry >> 9;
        d->nbits -= entry >> 9;

        return entry & 0x1FF;

    }

    for (i = 1; i < COUNTS; i++)
    {

        code |= getbits(d, 1);

        if (code < first + h->count[i])
            return h->symbol[index + code - first];

        index += h->count[i];
        first = (first + h->count[i]) << 1;
        code <<= 1;

    }

    d->status = DEFLATE_INVALID;

    return 0;

}

static void stored(struct deflate *d)
{

    unsigned int length;
    unsigned int check;

    d->bits >>= d->nbits & 7;
    d->nbits -= d->nbits & 7;
    length = getbits(d, 8);
    length |= getbits(d, 8) << 8;
    check = getbits(d, 8);
    check |= getbits(d, 8) << 8;

    if (d->status != DEFLATE_OK)
        return;

    if (length != (~check & 0xFFFF))
    {

        d->status = DEFLATE_INVALID;

        return;

    }

    d->block = BLOCK_STORED;
    d->length = length;

}

static void symbol(struct deflate *d)
{

    unsigned int symbol = decode(d, &d->literals);
    unsigned int length;
    unsigned int distance;

    if (d->status != DEFLATE_OK)
        return;

    if (symbol < 256)
    {

        putbyte(d, symbol);

        return;

    }

    if (symbol == 256)
    {

        d->block = BLOCK_NONE;

        return;

    }

    symbol -= 257;

    if (symbol >= LENGTHS)
    {

        d->status = DEFLATE_INVALID;

        return;

    }

    length = lengthbase[symbol] + getbits(d, lengthextra[symbol]);
    symbol = decode(d, &d->distances);

    if (symbol >= DISTANCES)
    {

        d->status = DEFLATE_INVALID;

        return;

    }

    distance = distancebase[symbol] + getbits(d, distanceextra[symbol]);

    if (d->status != DEFLATE_OK)
        return;

    if (distance > d->ooffset || (d->ring && distance > d->ocount))
    {

        d->status = DEFLATE_INVALID;

        return;

    }

    d->length = length;
    d->distance = distance;

}

static void fixed(struct deflate_huffman *literals, struct deflate_huffman *distances)
{

    unsigned short lengths[LITERALS];
    unsigned int i;

    for (i = 0; i < LITERALS; i++)
        lengths[i] = (i < 144) ? 8 : (i < 256) ? 9 : (i < 280) ? 7 : 8;

    build(literals, lengths, LITERALS);

    for (i = 0; i < DISTANCES; i++)
        lengths[i] = 5;

    build(distances, lengths, DISTANCES);

}

static void dynamic(struct deflate *d, struct deflate_huffman *literals, struct deflate_huffman *distances)
{

    unsigned short lengths[LITERALS + DISTANCES];
    unsigned int nliterals = getbits(d, 5) + 257;
    unsigned int ndistances = getbits(d, 5) + 1;
    unsigned int ncodelengths = getbits(d, 4) + 4;
    unsigned int i;

    if (nliterals > LITERALS - 2 || ndistances > DISTANCES)
    {

        d->status = DEFLATE_INVALID;

        return;

    }

    for (i = 0; i < CODELENGTHS; i++)
        lengths[codelengthorder[i]] = (i < ncodelengths) ? getbits(d, 3) : 0;

    if (!build(literals, lengths, CODELENGTHS))
        d->status = DEFLATE_INVALID;

    i = 0;

    while (i < nliterals + ndistances && d->status == DEFLATE_OK)
    {

        unsigned int symbol = decode(d, literals);
        unsigned int value = 0;
        unsigned int repeat;

        if (symbol < 16)
        {

            lengths[i++] = symbol;

        }

        else
        {

            if (symbol == 16)
            {

                if (!i)
                    d->status = DEFLATE_INVALID;
                else
                    value = lengths[i - 1];

                repeat = 3 + getbits(d, 2);

            }

            else if (symbol == 17)
            {

                repeat = 3 + getbits(d, 3);

            }

            else
            {

                repeat = 11 + getbits(d, 7);

            }

            if (i + repeat > nliterals + ndistances)
                d->status = DEFLATE_INVALID;

            while (repeat-- && d->status == DEFLATE_OK)
                lengths[i++] = value;

        }

    }

    if (d->status != DEFLATE_OK)
        return;

    if (!lengths[256] || !build(literals, lengths, nliterals) || !build(distances, lengths + nliterals, ndistances))
        d->status = DEFLATE_INVALID;

}

static void header(struct deflate *d)
{

    d->last = getbits(d, 1);

    switch (getbits(d, 2))
    {

    case 0:
        stored(d);

        break;

    case 1:
        fixed(&d->literals, &d->distances);

        d->block = BLOCK_CODES;

        break;

    case 2:
        dynamic(d, &d->literals, &d->distances);

        d->block = BLOCK_CODES;

        break;

    default:
        d->status = DEFLATE_INVALID;

        break;

    }

}

/* decompresses until count more bytes are out or the stream ends, and returns how many came out; it stops between any two bytes, inside a block or a match */
unsigned int deflate_inflate(struct deflate *d, unsigned int count)
{

    unsigned int start = d->ooffset;

    while (d->ooffset - start < count && d->status == DEFLATE_OK)
    {

        if (d->length)
        {

            d->length--;

            putbyte(d, (d->block == BLOCK_STORED) ? getbits(d, 8) : getwindow(d, d->distance));

            continue;

        }

        switch (d->block)
        {

        case BLOCK_NONE:
            if (d->last)
            {

                giveback(d);

                return d->ooffset - start;

            }

            header(d);

            break;

        case BLOCK_STORED:
            d->block = BLOCK_NONE;

            break;

        case BLOCK_CODES:
            symbol(d);

            break;

        }

    }

    return d->ooffset - start;

}

unsigned int deflate_decompress(struct deflate *d)
{

    deflate_inflate(d, 0xFFFFFFFF);

    return d->status;

}

void deflate_init(struct deflate *d, void *out, unsigned int ocount, void *in, unsigned int icount)
{

    d->out = out;
    d->ocount = ocount;
    d->ooffset = 0;
    d->in = in;
    d->icount = icount;
    d->ioffset = 0;
    d->bits = 0;
    d->nbits = 0;
    d->status = DEFLATE_OK;
    d->ring = 0;
    d->refill = 0;
    d->last = 0;
    d->block = BLOCK_NONE;
    d->length = 0;
    d->distance = 0;

}

void deflate_initring(struct deflate *d, void *window, unsigned int count, unsigned int (*refill)(struct deflate *d))
{

    deflate_init(d, window, count, 0, 0);

    d->ring = 1;
    d->refill = refill;

}
