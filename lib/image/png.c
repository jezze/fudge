#include <compression/deflate.h>
#include "png.h"

#define SIGNATURESIZE                   8
#define HEADERSIZE                      (SIGNATURESIZE + 25)

static unsigned char signature[SIGNATURESIZE] = {0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A};

static unsigned int read16(unsigned char *data)
{

    return (data[0] << 8) | data[1];

}

static unsigned int read32(unsigned char *data)
{

    return ((unsigned int)data[0] << 24) | (data[1] << 16) | (data[2] << 8) | data[3];

}

static unsigned int istype(unsigned char *data, char *type)
{

    return data[0] == type[0] && data[1] == type[1] && data[2] == type[2] && data[3] == type[3];

}

static unsigned int paeth(unsigned int a, unsigned int b, unsigned int c)
{

    int p = a + b - c;
    int pa = (p > (int)a) ? p - a : a - p;
    int pb = (p > (int)b) ? p - b : b - p;
    int pc = (p > (int)c) ? p - c : c - p;

    if (pa <= pb && pa <= pc)
        return a;

    return (pb <= pc) ? b : c;

}

/* the line holds the row above until each byte is replaced, so up keeps the bytes of the row above that the pixel to the left still needs */
static unsigned int unfilter(struct png *png, unsigned int start)
{

    unsigned char *window = png->window;
    unsigned int mask = PNG_WINDOW - 1;
    unsigned int filter = window[start & mask];
    unsigned char up[8];
    unsigned int j = 0;
    unsigned int i;

    if (filter > 4)
        return 0;

    for (i = 0; i < png->stride; i++)
    {

        unsigned int value = window[(start + 1 + i) & mask];
        unsigned int a = (i >= png->pixelsize) ? png->line[i - png->pixelsize] : 0;
        unsigned int b = png->line[i];
        unsigned int c = (i >= png->pixelsize) ? up[j] : 0;

        up[j] = b;
        j = (j + 1 == png->pixelsize) ? 0 : j + 1;

        switch (filter)
        {

        case 1:
            value += a;

            break;

        case 2:
            value += b;

            break;

        case 3:
            value += (a + b) / 2;

            break;

        case 4:
            value += paeth(a, b, c);

            break;

        }

        png->line[i] = value;

    }

    return 1;

}

/* the IDAT payloads are handed to deflate one after another, skipping the CRC and the next chunk's header in between */
static unsigned int refill(struct deflate *d)
{

    struct png *png = (struct png *)d;
    unsigned char header[8];
    unsigned int count;

    while (!png->left)
    {

        if (png->read(png, header, 8, png->offset + 4) != 8 || !istype(header + 4, "IDAT"))
            return 0;

        png->left = read32(header);
        png->offset += 12;

    }

    count = png->read(png, png->input, (png->left < PNG_INPUT) ? png->left : PNG_INPUT, png->offset);

    png->offset += count;
    png->left -= count;
    d->in = png->input;
    d->icount = count;
    d->ioffset = 0;

    return count;

}

static void rewind(struct png *png)
{

    unsigned int i;

    deflate_initring(&png->deflate, png->window, PNG_WINDOW, refill);

    png->offset = png->start;
    png->left = png->startleft;
    png->row = 0;

    for (i = 0; i < png->stride; i++)
        png->line[i] = 0;

}

unsigned int png_readheader(struct png *png, void *in, unsigned int icount)
{

    unsigned char *data = in;
    unsigned int i;

    if (icount < HEADERSIZE)
        return 0;

    for (i = 0; i < SIGNATURESIZE; i++)
    {

        if (data[i] != signature[i])
            return 0;

    }

    if (read32(data + 8) != 13 || !istype(data + 12, "IHDR"))
        return 0;

    png->width = read32(data + 16);
    png->height = read32(data + 20);
    png->depth = data[24];
    png->colortype = data[25];

    switch (png->colortype)
    {

    case 0:
        png->channels = (png->depth == 1 || png->depth == 2 || png->depth == 4 || png->depth == 8 || png->depth == 16) ? 1 : 0;

        break;

    case 2:
        png->channels = (png->depth == 8 || png->depth == 16) ? 3 : 0;

        break;

    case 3:
        png->channels = (png->depth == 1 || png->depth == 2 || png->depth == 4 || png->depth == 8) ? 1 : 0;

        break;

    case 4:
        png->channels = (png->depth == 8 || png->depth == 16) ? 2 : 0;

        break;

    case 6:
        png->channels = (png->depth == 8 || png->depth == 16) ? 4 : 0;

        break;

    default:
        png->channels = 0;

        break;

    }

    /* compression and filter method 0, no interlace */
    if (!png->channels || data[26] || data[27] || data[28])
        return 0;

    if (!png->width || !png->height || png->width > 0x1000000)
        return 0;

    png->pixelsize = (png->channels * png->depth + 7) / 8;
    png->stride = (png->width * png->channels * png->depth + 7) / 8;

    png->npalette = 0;
    png->haskey = 0;

    for (i = 0; i < PNG_PALETTE; i++)
        png->palette[i] = 0xFF000000;

    return 1;

}

/* reads the chunks before the first IDAT: PLTE and tRNS are kept, the rest is skipped */
unsigned int png_open(struct png *png, unsigned int (*read)(struct png *png, void *buffer, unsigned int count, unsigned int offset))
{

    unsigned char *data = png->input;
    unsigned int offset = SIGNATURESIZE;
    unsigned int i;

    png->read = read;

    if (read(png, data, HEADERSIZE, 0) != HEADERSIZE || !png_readheader(png, data, HEADERSIZE) || png->stride > PNG_LINE)
        return 0;

    while (read(png, data, 8, offset) == 8)
    {

        unsigned int length = read32(data);
        unsigned int plte = istype(data + 4, "PLTE");
        unsigned int trns = istype(data + 4, "tRNS");

        if (istype(data + 4, "IDAT"))
        {

            if (length < 2 || read(png, data, 2, offset + 8) != 2 || (data[0] & 0x0F) != 8 || (data[0] * 256 + data[1]) % 31 || data[1] & 0x20)
                return 0;

            if (png->colortype == 3 && !png->npalette)
                return 0;

            png->start = offset + 10;
            png->startleft = length - 2;

            rewind(png);

            return 1;

        }

        if (istype(data + 4, "IEND"))
            return 0;

        if (plte || trns)
        {

            if (length > PNG_INPUT || read(png, data, length, offset + 8) != length)
                return 0;

        }

        if (plte)
        {

            if (length % 3 || length / 3 > PNG_PALETTE)
                return 0;

            png->npalette = length / 3;

            for (i = 0; i < png->npalette; i++)
                png->palette[i] = 0xFF000000 | (data[i * 3] << 16) | (data[i * 3 + 1] << 8) | data[i * 3 + 2];

        }

        if (trns)
        {

            if (png->colortype == 3)
            {

                for (i = 0; i < length && i < png->npalette; i++)
                    png->palette[i] = (png->palette[i] & 0x00FFFFFF) | ((unsigned int)data[i] << 24);

            }

            if (png->colortype == 0 && length >= 2)
            {

                png->key[0] = read16(data);
                png->haskey = 1;

            }

            if (png->colortype == 2 && length >= 6)
            {

                png->key[0] = read16(data);
                png->key[1] = read16(data + 2);
                png->key[2] = read16(data + 4);
                png->haskey = 1;

            }

        }

        if (length > 0xFFFFFFFF - offset - 12)
            return 0;

        offset += length + 12;

    }

    return 0;

}

/* rows are decoded in order and the last one is kept, as with pcx; a row above it starts over from the first IDAT */
static unsigned int decoderow(struct png *png, unsigned int row)
{

    if (row + 1 < png->row)
        rewind(png);

    while (png->row <= row)
    {

        if (deflate_inflate(&png->deflate, png->stride + 1) != png->stride + 1 || !unfilter(png, png->deflate.ooffset - png->stride - 1))
            return 0;

        png->row++;

    }

    return 1;

}

static unsigned int getsample(unsigned char *line, unsigned int index, unsigned int depth)
{

    unsigned int bit;

    switch (depth)
    {

    case 16:
        return read16(line + index * 2);

    case 8:
        return line[index];

    }

    bit = index * depth;

    return (line[bit / 8] >> (8 - depth - bit % 8)) & ((1 << depth) - 1);

}

static unsigned int scale(unsigned int value, unsigned int depth)
{

    switch (depth)
    {

    case 16:
        return value >> 8;

    case 8:
        return value;

    }

    return value * 255 / ((1 << depth) - 1);

}

unsigned int png_readrow(struct png *png, unsigned int row, unsigned int *pixels)
{

    unsigned char *line = png->line;
    unsigned int x;

    if (row >= png->height || !decoderow(png, row))
        return 0;

    for (x = 0; x < png->width; x++)
    {

        unsigned int index = x * png->channels;
        unsigned int s0 = getsample(line, index, png->depth);
        unsigned int v0 = scale(s0, png->depth);

        switch (png->colortype)
        {

        case 0:
            pixels[x] = ((png->haskey && s0 == png->key[0]) ? 0 : 0xFF000000) | (v0 << 16) | (v0 << 8) | v0;

            break;

        case 2:
            {

                unsigned int s1 = getsample(line, index + 1, png->depth);
                unsigned int s2 = getsample(line, index + 2, png->depth);
                unsigned int transparent = png->haskey && s0 == png->key[0] && s1 == png->key[1] && s2 == png->key[2];

                pixels[x] = ((transparent) ? 0 : 0xFF000000) | (v0 << 16) | (scale(s1, png->depth) << 8) | scale(s2, png->depth);

            }

            break;

        case 3:
            pixels[x] = png->palette[s0];

            break;

        case 4:
            pixels[x] = (scale(getsample(line, index + 1, png->depth), png->depth) << 24) | (v0 << 16) | (v0 << 8) | v0;

            break;

        case 6:
            pixels[x] = (scale(getsample(line, index + 3, png->depth), png->depth) << 24) | (v0 << 16) | (scale(getsample(line, index + 1, png->depth), png->depth) << 8) | scale(getsample(line, index + 2, png->depth), png->depth);

            break;

        }

    }

    return 1;

}
