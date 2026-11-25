#include <fudge.h>
#include "jpeg.h"

#define END                             0x100

struct part
{

    int x0;
    int x1;
    int x2;
    int x3;
    int t0;
    int t1;
    int t2;
    int t3;

};

static unsigned char zigzag[JPEG_BLOCK] = {
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

static unsigned int nextbyte(struct jpeg *jpeg)
{

    if (jpeg->offset >= jpeg->inputstart + jpeg->inputcount)
    {

        jpeg->inputstart = jpeg->offset;
        jpeg->inputcount = jpeg->read(jpeg, jpeg->input, JPEG_INPUT, jpeg->offset);

        if (!jpeg->inputcount)
            return END;

    }

    return jpeg->input[jpeg->offset++ - jpeg->inputstart];

}

static unsigned int read16(struct jpeg *jpeg)
{

    unsigned int high = nextbyte(jpeg);
    unsigned int low = nextbyte(jpeg);

    return (high == END || low == END) ? 0 : (high << 8) | low;

}

static unsigned int findmarker(struct jpeg *jpeg)
{

    unsigned int c = nextbyte(jpeg);

    while (c != END)
    {

        if (c == 0xFF)
        {

            c = nextbyte(jpeg);

            if (c != 0xFF && c != 0)
                return c;

        }

        else
        {

            c = nextbyte(jpeg);

        }

    }

    return END;

}

static unsigned int readframe(struct jpeg *jpeg)
{

    unsigned int length = read16(jpeg);
    unsigned int precision = nextbyte(jpeg);
    unsigned int i;

    jpeg->height = read16(jpeg);
    jpeg->width = read16(jpeg);
    jpeg->ncomponents = nextbyte(jpeg);

    if (precision != 8 || !jpeg->width || !jpeg->height || jpeg->width > JPEG_WIDTH || (jpeg->ncomponents != 1 && jpeg->ncomponents != JPEG_COMPONENTS) || length != 8 + jpeg->ncomponents * 3)
        return 0;

    jpeg->hmax = 1;
    jpeg->vmax = 1;

    for (i = 0; i < jpeg->ncomponents; i++)
    {

        struct jpeg_component *component = &jpeg->components[i];
        unsigned int sampling;

        component->id = nextbyte(jpeg);
        sampling = nextbyte(jpeg);
        component->quant = nextbyte(jpeg);
        component->h = (jpeg->ncomponents == 1) ? 1 : sampling >> 4;
        component->v = (jpeg->ncomponents == 1) ? 1 : sampling & 0x0F;

        /* only factors of one and two, which is every subsampling seen on the web */
        if (component->h < 1 || component->h > 2 || component->v < 1 || component->v > 2 || component->quant >= JPEG_TABLES)
            return 0;

        jpeg->hmax = (component->h > jpeg->hmax) ? component->h : jpeg->hmax;
        jpeg->vmax = (component->v > jpeg->vmax) ? component->v : jpeg->vmax;

    }

    jpeg->mcuwidth = jpeg->hmax * 8;
    jpeg->mcuheight = jpeg->vmax * 8;
    jpeg->mcus = (jpeg->width + jpeg->mcuwidth - 1) / jpeg->mcuwidth;

    jpeg->mcurows = (jpeg->height + jpeg->mcuheight - 1) / jpeg->mcuheight;
    jpeg->needed = 0;

    for (i = 0; i < jpeg->ncomponents; i++)
    {

        struct jpeg_component *component = &jpeg->components[i];

        component->blocks = jpeg->mcus * component->h;
        component->rows = jpeg->mcurows * component->v;
        component->stride = component->blocks * 8;
        component->offset = jpeg->needed / 2;
        jpeg->needed += component->blocks * component->rows * JPEG_BLOCK * 2;

    }

    return 1;

}

static unsigned int readquant(struct jpeg *jpeg)
{

    int left = (int)read16(jpeg) - 2;

    while (left > 0)
    {

        unsigned int info = nextbyte(jpeg);
        unsigned int wide = info >> 4;
        unsigned int table = info & 0x0F;
        unsigned int i;

        if (info == END || table >= JPEG_TABLES)
            return 0;

        for (i = 0; i < JPEG_BLOCK; i++)
            jpeg->quant[table][i] = (wide) ? read16(jpeg) : nextbyte(jpeg);

        left -= 1 + JPEG_BLOCK * ((wide) ? 2 : 1);

    }

    return left == 0;

}

static unsigned int readtable(struct jpeg *jpeg, struct jpeg_huffman *table)
{

    unsigned char counts[JPEG_LENGTHS - 1];
    unsigned int total = 0;
    unsigned int code = 0;
    unsigned int index = 0;
    unsigned int length;
    unsigned int i;

    for (i = 0; i < JPEG_LENGTHS - 1; i++)
    {

        counts[i] = nextbyte(jpeg);
        total += counts[i];

    }

    if (total > JPEG_SYMBOLS)
        return 0;

    for (i = 0; i < total; i++)
        table->values[i] = nextbyte(jpeg);

    buffer_clear(table->fast, sizeof (unsigned short) * JPEG_FAST);

    for (length = 1; length < JPEG_LENGTHS; length++)
    {

        unsigned int count = counts[length - 1];

        table->first[length] = index;
        table->mincode[length] = code;
        table->maxcode[length] = (count) ? (int)(code + count - 1) : -1;

        for (i = 0; i < count; i++)
        {

            if (length <= JPEG_FASTBITS)
            {

                unsigned int start = code << (JPEG_FASTBITS - length);
                unsigned int span = 1 << (JPEG_FASTBITS - length);
                unsigned int j;

                for (j = 0; j < span && start + j < JPEG_FAST; j++)
                    table->fast[start + j] = (length << 8) | table->values[index];

            }

            code++;
            index++;

        }

        code <<= 1;

    }

    return 1 + (JPEG_LENGTHS - 1) + total;

}

static unsigned int readhuffman(struct jpeg *jpeg)
{

    int left = (int)read16(jpeg) - 2;

    while (left > 0)
    {

        unsigned int info = nextbyte(jpeg);
        unsigned int table = info & 0x0F;
        unsigned int count;

        if (info == END || table >= JPEG_TABLES)
            return 0;

        count = readtable(jpeg, (info >> 4) ? &jpeg->ac[table] : &jpeg->dc[table]);

        if (!count)
            return 0;

        left -= count;

    }

    return left == 0;

}

static void readadobe(struct jpeg *jpeg)
{

    unsigned int length = read16(jpeg);
    unsigned char data[12];
    unsigned int i;

    for (i = 0; i + 2 < length; i++)
    {

        unsigned int c = nextbyte(jpeg);

        if (i < 12)
            data[i] = c;

    }

    if (length >= 14 && buffer_match(data, "Adobe", 5))
        jpeg->transform = data[11];

}

static void skipsegment(struct jpeg *jpeg)
{

    unsigned int length = read16(jpeg);

    if (length >= 2)
        jpeg->offset += length - 2;

}

static void resetscan(struct jpeg *jpeg)
{

    unsigned int i;

    for (i = 0; i < jpeg->ncomponents; i++)
        jpeg->components[i].predictor = 0;

    jpeg->bits = 0;
    jpeg->bitcount = 0;
    jpeg->marker = 0;
    jpeg->eobrun = 0;
    jpeg->restartleft = jpeg->restart;

}

static unsigned int readscan(struct jpeg *jpeg)
{

    unsigned int length = read16(jpeg);
    unsigned int count = nextbyte(jpeg);
    unsigned int approximation;
    unsigned int i;

    if (!jpeg->ncomponents || !count || count > jpeg->ncomponents || length != 6 + count * 2)
        return 0;

    for (i = 0; i < count; i++)
    {

        unsigned int id = nextbyte(jpeg);
        unsigned int tables = nextbyte(jpeg);
        unsigned int j;

        jpeg->scan[i] = JPEG_COMPONENTS;

        for (j = 0; j < jpeg->ncomponents; j++)
        {

            if (jpeg->components[j].id == id)
            {

                jpeg->components[j].dc = (tables >> 4) & 3;
                jpeg->components[j].ac = tables & 3;
                jpeg->scan[i] = j;

            }

        }

        if (jpeg->scan[i] == JPEG_COMPONENTS)
            return 0;

    }

    jpeg->scancount = count;
    jpeg->start = nextbyte(jpeg);
    jpeg->end = nextbyte(jpeg);
    approximation = nextbyte(jpeg);
    jpeg->high = approximation >> 4;
    jpeg->low = approximation & 0x0F;

    if (jpeg->start > jpeg->end || jpeg->end >= JPEG_BLOCK || jpeg->high > 13 || jpeg->low > 13)
        return 0;

    if (jpeg->progressive && ((!jpeg->start && jpeg->end) || (jpeg->start && count != 1)))
        return 0;

    if (!jpeg->progressive && (jpeg->start || jpeg->end != JPEG_BLOCK - 1 || approximation))
        return 0;

    resetscan(jpeg);

    return 1;

}

static void fillbits(struct jpeg *jpeg)
{

    while (jpeg->bitcount <= 24)
    {

        unsigned int c = 0;

        if (!jpeg->marker)
        {

            c = nextbyte(jpeg);

            if (c == 0xFF)
            {

                unsigned int next = nextbyte(jpeg);

                while (next == 0xFF)
                    next = nextbyte(jpeg);

                if (next)
                {

                    jpeg->marker = next;
                    c = 0;

                }

            }

            else if (c == END)
            {

                jpeg->marker = END;
                c = 0;

            }

        }

        jpeg->bits |= c << (24 - jpeg->bitcount);
        jpeg->bitcount += 8;

    }

}

static void consume(struct jpeg *jpeg, unsigned int count)
{

    jpeg->bits <<= count;
    jpeg->bitcount -= count;

}

static unsigned int getbits(struct jpeg *jpeg, unsigned int count)
{

    unsigned int value;

    if (!count)
        return 0;

    fillbits(jpeg);

    value = jpeg->bits >> (32 - count);

    consume(jpeg, count);

    return value;

}

static unsigned int decode(struct jpeg *jpeg, struct jpeg_huffman *table)
{

    unsigned int fast;
    unsigned int length;

    fillbits(jpeg);

    fast = table->fast[jpeg->bits >> (32 - JPEG_FASTBITS)];

    if (fast)
    {

        consume(jpeg, fast >> 8);

        return fast & 0xFF;

    }

    for (length = JPEG_FASTBITS + 1; length < JPEG_LENGTHS; length++)
    {

        unsigned int code = jpeg->bits >> (32 - length);

        if ((int)code <= table->maxcode[length])
        {

            consume(jpeg, length);

            return table->values[(table->first[length] + code - table->mincode[length]) & 0xFF];

        }

    }

    return 0;

}

static int extend(unsigned int value, unsigned int size)
{

    if (!size)
        return 0;

    return (value < (1u << (size - 1))) ? (int)value - (1 << size) + 1 : (int)value;

}

static int limit(int value, int bound)
{

    return (value < -bound) ? -bound : (value > bound) ? bound : value;

}

static int dequantize(int value, unsigned int quant)
{

    /* an 8 bit image never has a coefficient outside 12 bits, a damaged one is kept from overflowing the transform */
    return limit(limit(value, 32767) * (int)quant, 2047);

}

static void decodeblock(struct jpeg *jpeg, struct jpeg_component *component)
{

    unsigned int size = decode(jpeg, &jpeg->dc[component->dc]) & 0x0F;
    unsigned int k;

    buffer_clear(jpeg->block, sizeof (int) * JPEG_BLOCK);

    component->predictor = limit(component->predictor + extend(getbits(jpeg, size), size), 32767);
    jpeg->block[0] = dequantize(component->predictor, jpeg->quant[component->quant][0]);

    for (k = 1; k < JPEG_BLOCK;)
    {

        unsigned int symbol = decode(jpeg, &jpeg->ac[component->ac]);
        unsigned int run = symbol >> 4;

        size = symbol & 0x0F;

        if (!size)
        {

            if (run != 15)
                break;

            k += 16;

            continue;

        }

        k += run;

        if (k >= JPEG_BLOCK)
            break;

        jpeg->block[zigzag[k]] = dequantize(extend(getbits(jpeg, size), size), jpeg->quant[component->quant][k]);
        k++;

    }

}

static void setcoefficient(struct jpeg *jpeg, unsigned int index, int value)
{

    jpeg->coefficients[index] = limit(value, 32767);

}

static void decodedcfirst(struct jpeg *jpeg, struct jpeg_component *component, unsigned int offset)
{

    unsigned int size = decode(jpeg, &jpeg->dc[component->dc]) & 0x0F;

    component->predictor = limit(component->predictor + extend(getbits(jpeg, size), size), 32767);

    setcoefficient(jpeg, offset, component->predictor * (1 << jpeg->low));

}

static void decodedcrefine(struct jpeg *jpeg, unsigned int offset)
{

    if (getbits(jpeg, 1))
        jpeg->coefficients[offset] |= 1 << jpeg->low;

}

static void decodeacfirst(struct jpeg *jpeg, struct jpeg_component *component, unsigned int offset, unsigned int start)
{

    unsigned int k;

    if (jpeg->eobrun)
    {

        jpeg->eobrun--;

        return;

    }

    for (k = start; k <= jpeg->end; k++)
    {

        unsigned int symbol = decode(jpeg, &jpeg->ac[component->ac]);
        unsigned int run = symbol >> 4;
        unsigned int size = symbol & 0x0F;

        if (!size)
        {

            /* a run below 15 ends this block and maybe the ones after it */
            if (run < 15)
            {

                jpeg->eobrun = (1 << run) - 1 + getbits(jpeg, run);

                break;

            }

            k += 15;

            continue;

        }

        k += run;

        if (k > jpeg->end)
            break;

        setcoefficient(jpeg, offset + zigzag[k], extend(getbits(jpeg, size), size) * (1 << jpeg->low));

    }

}

static void refine(struct jpeg *jpeg, unsigned int index)
{

    int bit = 1 << jpeg->low;
    int value = jpeg->coefficients[index];

    if (getbits(jpeg, 1) && !(value & bit))
        setcoefficient(jpeg, index, (value >= 0) ? value + bit : value - bit);

}

static void decodeacrefine(struct jpeg *jpeg, struct jpeg_component *component, unsigned int offset)
{

    unsigned int k = jpeg->start;

    if (!jpeg->eobrun)
    {

        for (; k <= jpeg->end; k++)
        {

            unsigned int symbol = decode(jpeg, &jpeg->ac[component->ac]);
            int run = symbol >> 4;
            int value = 0;

            if (symbol & 0x0F)
            {

                value = (getbits(jpeg, 1)) ? 1 << jpeg->low : -(1 << jpeg->low);

            }

            else if (run != 15)
            {

                jpeg->eobrun = (1 << run) + getbits(jpeg, run);

                break;

            }

            /* coefficients already set get a correction bit, zeros count down the run */
            while (k <= jpeg->end)
            {

                unsigned int index = offset + zigzag[k];

                if (jpeg->coefficients[index])
                {

                    refine(jpeg, index);

                }

                else
                {

                    if (!run)
                        break;

                    run--;

                }

                k++;

            }

            if (value && k <= jpeg->end)
                setcoefficient(jpeg, offset + zigzag[k], value);

        }

    }

    if (jpeg->eobrun)
    {

        for (; k <= jpeg->end; k++)
        {

            unsigned int index = offset + zigzag[k];

            if (jpeg->coefficients[index])
                refine(jpeg, index);

        }

        jpeg->eobrun--;

    }

}

static void decodeunit(struct jpeg *jpeg, struct jpeg_component *component, unsigned int block)
{

    unsigned int offset = component->offset + block * JPEG_BLOCK;

    if (!jpeg->progressive)
    {

        decodedcfirst(jpeg, component, offset);
        decodeacfirst(jpeg, component, offset, 1);

    }

    else if (!jpeg->start)
    {

        if (jpeg->high)
            decodedcrefine(jpeg, offset);
        else
            decodedcfirst(jpeg, component, offset);

    }

    else
    {

        if (jpeg->high)
            decodeacrefine(jpeg, component, offset);
        else
            decodeacfirst(jpeg, component, offset, jpeg->start);

    }

}

static struct part idctpart(int s0, int s1, int s2, int s3, int s4, int s5, int s6, int s7)
{

    struct part part;
    int p1 = (s2 + s6) * 2217;
    int p2;
    int p3;
    int p4;
    int p5;
    int t0;
    int t1;
    int t2;
    int t3;

    t2 = p1 - s6 * 7568;
    t3 = p1 + s2 * 3135;
    t0 = (s0 + s4) * 4096;
    t1 = (s0 - s4) * 4096;
    part.x0 = t0 + t3;
    part.x3 = t0 - t3;
    part.x1 = t1 + t2;
    part.x2 = t1 - t2;
    p3 = s7 + s3;
    p4 = s5 + s1;
    p1 = s7 + s1;
    p2 = s5 + s3;
    p5 = (p3 + p4) * 4816;
    t0 = s7 * 1223;
    t1 = s5 * 8410;
    t2 = s3 * 12586;
    t3 = s1 * 6149;
    p1 = p5 - p1 * 3686;
    p2 = p5 - p2 * 10498;
    p3 = p3 * -8035;
    p4 = p4 * -1598;
    part.t3 = t3 + p1 + p4;
    part.t2 = t2 + p2 + p3;
    part.t1 = t1 + p2 + p4;
    part.t0 = t0 + p1 + p3;

    return part;

}

static unsigned char clamp(int value)
{

    return (value < 0) ? 0 : (value > 255) ? 255 : value;

}

static void idct(struct jpeg *jpeg, unsigned int index, unsigned int base)
{

    unsigned int stride = jpeg->components[index].stride;
    int temp[JPEG_BLOCK];
    unsigned int i;

    for (i = 0; i < 8; i++)
    {

        if (!jpeg->block[i + 8] && !jpeg->block[i + 16] && !jpeg->block[i + 24] && !jpeg->block[i + 32] && !jpeg->block[i + 40] && !jpeg->block[i + 48] && !jpeg->block[i + 56])
        {

            int dc = jpeg->block[i] * 4;
            unsigned int j;

            for (j = 0; j < 8; j++)
                temp[i + j * 8] = dc;

        }

        else
        {

            struct part part = idctpart(jpeg->block[i], jpeg->block[i + 8], jpeg->block[i + 16], jpeg->block[i + 24], jpeg->block[i + 32], jpeg->block[i + 40], jpeg->block[i + 48], jpeg->block[i + 56]);

            part.x0 += 512;
            part.x1 += 512;
            part.x2 += 512;
            part.x3 += 512;
            temp[i] = (part.x0 + part.t3) >> 10;
            temp[i + 56] = (part.x0 - part.t3) >> 10;
            temp[i + 8] = (part.x1 + part.t2) >> 10;
            temp[i + 48] = (part.x1 - part.t2) >> 10;
            temp[i + 16] = (part.x2 + part.t1) >> 10;
            temp[i + 40] = (part.x2 - part.t1) >> 10;
            temp[i + 24] = (part.x3 + part.t0) >> 10;
            temp[i + 32] = (part.x3 - part.t0) >> 10;

        }

    }

    /* the rows also add the rounding and the level shift of 128 */
    for (i = 0; i < 8; i++)
    {

        unsigned int row = i * 8;
        unsigned int out = base + i * stride;
        struct part part = idctpart(temp[row], temp[row + 1], temp[row + 2], temp[row + 3], temp[row + 4], temp[row + 5], temp[row + 6], temp[row + 7]);
        int bias = 65536 + (128 << 17);

        part.x0 += bias;
        part.x1 += bias;
        part.x2 += bias;
        part.x3 += bias;
        jpeg->planes[index][out] = clamp((part.x0 + part.t3) >> 17);
        jpeg->planes[index][out + 7] = clamp((part.x0 - part.t3) >> 17);
        jpeg->planes[index][out + 1] = clamp((part.x1 + part.t2) >> 17);
        jpeg->planes[index][out + 6] = clamp((part.x1 - part.t2) >> 17);
        jpeg->planes[index][out + 2] = clamp((part.x2 + part.t1) >> 17);
        jpeg->planes[index][out + 5] = clamp((part.x2 - part.t1) >> 17);
        jpeg->planes[index][out + 3] = clamp((part.x3 + part.t0) >> 17);
        jpeg->planes[index][out + 4] = clamp((part.x3 - part.t0) >> 17);

    }

}

static void restart(struct jpeg *jpeg)
{

    while (!jpeg->marker)
    {

        unsigned int c = nextbyte(jpeg);

        if (c == END)
        {

            jpeg->marker = END;

        }

        else if (c == 0xFF)
        {

            c = nextbyte(jpeg);

            if (c != 0 && c != 0xFF)
                jpeg->marker = c;

        }

    }

    /* a restart marker starts the next interval, anything else ends the data */
    if (jpeg->marker >= 0xD0 && jpeg->marker <= 0xD7)
        resetscan(jpeg);

}

static void nextunit(struct jpeg *jpeg)
{

    if (jpeg->restart)
    {

        if (!jpeg->restartleft)
            restart(jpeg);

        jpeg->restartleft--;

    }

}

static void decoderow(struct jpeg *jpeg)
{

    unsigned int mcu;

    for (mcu = 0; mcu < jpeg->mcus; mcu++)
    {

        unsigned int i;

        nextunit(jpeg);

        for (i = 0; i < jpeg->ncomponents; i++)
        {

            struct jpeg_component *component = &jpeg->components[i];
            unsigned int y;

            for (y = 0; y < component->v; y++)
            {

                unsigned int x;

                for (x = 0; x < component->h; x++)
                {

                    decodeblock(jpeg, component);
                    idct(jpeg, i, y * 8 * component->stride + (mcu * component->h + x) * 8);

                }

            }

        }

    }

}

static void decodescan(struct jpeg *jpeg)
{

    unsigned int y;

    if (jpeg->scancount == 1)
    {

        /* a scan of one component goes over just the blocks the image covers */
        struct jpeg_component *component = &jpeg->components[jpeg->scan[0]];
        unsigned int width = ((jpeg->width * component->h + jpeg->hmax - 1) / jpeg->hmax + 7) / 8;
        unsigned int height = ((jpeg->height * component->v + jpeg->vmax - 1) / jpeg->vmax + 7) / 8;

        for (y = 0; y < height; y++)
        {

            unsigned int x;

            for (x = 0; x < width; x++)
            {

                nextunit(jpeg);
                decodeunit(jpeg, component, y * component->blocks + x);

            }

        }

        return;

    }

    for (y = 0; y < jpeg->mcurows; y++)
    {

        unsigned int mcu;

        for (mcu = 0; mcu < jpeg->mcus; mcu++)
        {

            unsigned int i;

            nextunit(jpeg);

            for (i = 0; i < jpeg->scancount; i++)
            {

                struct jpeg_component *component = &jpeg->components[jpeg->scan[i]];
                unsigned int by;

                for (by = 0; by < component->v; by++)
                {

                    unsigned int bx;

                    for (bx = 0; bx < component->h; bx++)
                        decodeunit(jpeg, component, (y * component->v + by) * component->blocks + mcu * component->h + bx);

                }

            }

        }

    }

}

static void buildrow(struct jpeg *jpeg, unsigned int row)
{

    unsigned int i;

    for (i = 0; i < jpeg->ncomponents; i++)
    {

        struct jpeg_component *component = &jpeg->components[i];
        unsigned int y;

        for (y = 0; y < component->v; y++)
        {

            unsigned int x;

            for (x = 0; x < component->blocks; x++)
            {

                unsigned int offset = component->offset + ((row * component->v + y) * component->blocks + x) * JPEG_BLOCK;
                unsigned int k;

                for (k = 0; k < JPEG_BLOCK; k++)
                    jpeg->block[zigzag[k]] = dequantize(jpeg->coefficients[offset + zigzag[k]], jpeg->quant[component->quant][k]);

                idct(jpeg, i, y * 8 * component->stride + x * 8);

            }

        }

    }

}

static unsigned int sample(struct jpeg *jpeg, unsigned int index, unsigned int x, unsigned int y)
{

    struct jpeg_component *component = &jpeg->components[index];

    if (component->h < jpeg->hmax)
        x >>= 1;

    if (component->v < jpeg->vmax)
        y >>= 1;

    return jpeg->planes[index][y * component->stride + x];

}

static unsigned int readsegment(struct jpeg *jpeg, unsigned int marker)
{

    switch (marker)
    {

    case 0xC0:
    case 0xC1:
    case 0xC2:
        jpeg->progressive = marker == 0xC2;

        return !jpeg->ncomponents && readframe(jpeg);

    case 0xC4:
        return readhuffman(jpeg);

    case 0xDB:
        return readquant(jpeg);

    case 0xDD:
        read16(jpeg);

        jpeg->restart = read16(jpeg);

        return 1;

    case 0xEE:
        readadobe(jpeg);

        return 1;

    case 0xC3:
    case 0xC5:
    case 0xC6:
    case 0xC7:
    case 0xC9:
    case 0xCA:
    case 0xCB:
    case 0xCD:
    case 0xCE:
    case 0xCF:
    case 0xD9:
    case 0xDA:
    case END:
        return 0;

    }

    skipsegment(jpeg);

    return 1;

}

static unsigned int nextscan(struct jpeg *jpeg, unsigned int marker)
{

    while (marker != 0xDA)
    {

        if (!readsegment(jpeg, marker))
            return 0;

        marker = findmarker(jpeg);

    }

    return readscan(jpeg);

}

unsigned int jpeg_open(struct jpeg *jpeg, unsigned int (*read)(struct jpeg *jpeg, void *buffer, unsigned int count, unsigned int offset))
{

    jpeg->read = read;
    jpeg->offset = 0;
    jpeg->inputstart = 0;
    jpeg->inputcount = 0;
    jpeg->width = 0;
    jpeg->height = 0;
    jpeg->ncomponents = 0;
    jpeg->restart = 0;
    jpeg->transform = 1;
    jpeg->coefficients = 0;

    if (nextbyte(jpeg) != 0xFF || nextbyte(jpeg) != 0xD8 || !nextscan(jpeg, findmarker(jpeg)))
        return 0;

    jpeg->buffered = jpeg->progressive || jpeg->scancount != jpeg->ncomponents;
    jpeg->row = jpeg->mcuheight;
    jpeg->line = 0;

    if (!jpeg->buffered)
        jpeg->needed = 0;

    return 1;

}

unsigned int jpeg_decode(struct jpeg *jpeg, short *coefficients)
{

    if (!jpeg->buffered)
        return 1;

    jpeg->coefficients = coefficients;

    buffer_clear(coefficients, jpeg->needed);

    while (1)
    {

        unsigned int marker;

        decodescan(jpeg);

        /* the scan may have run into the next marker already */
        marker = (jpeg->marker && !(jpeg->marker >= 0xD0 && jpeg->marker <= 0xD7)) ? jpeg->marker : findmarker(jpeg);

        if (marker == 0xD9 || marker == END || !nextscan(jpeg, marker))
            break;

    }

    return 1;

}

unsigned int jpeg_readrow(struct jpeg *jpeg, unsigned int *pixels)
{

    unsigned int x;

    if (jpeg->line >= jpeg->height)
        return 0;

    if (jpeg->buffered && !jpeg->coefficients)
        return 0;

    if (jpeg->row >= jpeg->mcuheight)
    {

        if (jpeg->buffered)
            buildrow(jpeg, jpeg->line / jpeg->mcuheight);
        else
            decoderow(jpeg);

        jpeg->row = 0;

    }

    for (x = 0; x < jpeg->width; x++)
    {

        int y = sample(jpeg, 0, x, jpeg->row);

        if (jpeg->ncomponents == 1)
        {

            pixels[x] = 0xFF000000 | (y << 16) | (y << 8) | y;

        }

        else if (!jpeg->transform)
        {

            pixels[x] = 0xFF000000 | (y << 16) | (sample(jpeg, 1, x, jpeg->row) << 8) | sample(jpeg, 2, x, jpeg->row);

        }

        else
        {

            int cb = (int)sample(jpeg, 1, x, jpeg->row) - 128;
            int cr = (int)sample(jpeg, 2, x, jpeg->row) - 128;
            unsigned int r = clamp(y + ((91881 * cr + 32768) >> 16));
            unsigned int g = clamp(y + ((-22554 * cb - 46802 * cr + 32768) >> 16));
            unsigned int b = clamp(y + ((116130 * cb + 32768) >> 16));

            pixels[x] = 0xFF000000 | (r << 16) | (g << 8) | b;

        }

    }

    jpeg->row++;
    jpeg->line++;

    return 1;

}

