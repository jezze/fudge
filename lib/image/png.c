#include <compression/deflate.h>
#include "png.h"

#define SIGNATURESIZE                   8
#define HEADERSIZE                      (SIGNATURESIZE + 25)
#define ADLERBASE                       65521
#define ADLERBLOCK                      5552

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

static unsigned int adler32(unsigned char *data, unsigned int count)
{

    unsigned int a = 1;
    unsigned int b = 0;

    while (count)
    {

        unsigned int n = (count < ADLERBLOCK) ? count : ADLERBLOCK;

        count -= n;

        while (n--)
        {

            a += *data++;
            b += a;

        }

        a %= ADLERBASE;
        b %= ADLERBASE;

    }

    return (b << 16) | a;

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

static unsigned int unfilter(struct png *png)
{

    unsigned int row;

    for (row = 0; row < png->height; row++)
    {

        unsigned char *line = png->data + row * (png->stride + 1);
        unsigned char *current = line + 1;
        unsigned char *previous = (row) ? current - (png->stride + 1) : 0;
        unsigned int i;

        if (line[0] > 4)
            return 0;

        for (i = 0; i < png->stride; i++)
        {

            unsigned int a = (i >= png->pixelsize) ? current[i - png->pixelsize] : 0;
            unsigned int b = (previous) ? previous[i] : 0;
            unsigned int c = (previous && i >= png->pixelsize) ? previous[i - png->pixelsize] : 0;

            switch (line[0])
            {

            case 1:
                current[i] += a;

                break;

            case 2:
                current[i] += b;

                break;

            case 3:
                current[i] += (a + b) / 2;

                break;

            case 4:
                current[i] += paeth(a, b, c);

                break;

            }

        }

    }

    return 1;

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

    if (png->height > 0xFFFFFFFF / (png->stride + 1))
        return 0;

    png->size = png->height * (png->stride + 1);
    png->npalette = 0;
    png->haskey = 0;

    for (i = 0; i < PNG_PALETTE; i++)
        png->palette[i] = 0xFF000000;

    return 1;

}

unsigned int png_decode(struct png *png, void *out, unsigned int ocount, void *in, unsigned int icount)
{

    unsigned char *data = in;
    unsigned int offset = SIGNATURESIZE;
    unsigned int idat = 0;
    struct deflate d;
    unsigned int i;

    if (!png_readheader(png, in, icount) || ocount < png->size)
        return 0;

    while (offset + 12 <= icount)
    {

        unsigned int length = read32(data + offset);
        unsigned char *type = data + offset + 4;
        unsigned char *chunk = data + offset + 8;

        if (length > icount - offset - 12)
            return 0;

        if (istype(type, "IEND"))
            break;

        if (istype(type, "PLTE"))
        {

            if (length % 3 || length / 3 > PNG_PALETTE)
                return 0;

            png->npalette = length / 3;

            for (i = 0; i < png->npalette; i++)
                png->palette[i] = 0xFF000000 | (chunk[i * 3] << 16) | (chunk[i * 3 + 1] << 8) | chunk[i * 3 + 2];

        }

        if (istype(type, "tRNS"))
        {

            if (png->colortype == 3)
            {

                for (i = 0; i < length && i < png->npalette; i++)
                    png->palette[i] = (png->palette[i] & 0x00FFFFFF) | ((unsigned int)chunk[i] << 24);

            }

            if (png->colortype == 0 && length >= 2)
            {

                png->key[0] = read16(chunk);
                png->haskey = 1;

            }

            if (png->colortype == 2 && length >= 6)
            {

                png->key[0] = read16(chunk);
                png->key[1] = read16(chunk + 2);
                png->key[2] = read16(chunk + 4);
                png->haskey = 1;

            }

        }

        /* the IDAT payloads are moved together in place, since deflate needs the stream in one piece */
        if (istype(type, "IDAT"))
        {

            for (i = 0; i < length; i++)
                data[idat + i] = chunk[i];

            idat += length;

        }

        offset += length + 12;

    }

    if (png->colortype == 3 && !png->npalette)
        return 0;

    if (idat < 6 || (data[0] & 0x0F) != 8 || (data[0] * 256 + data[1]) % 31 || data[1] & 0x20)
        return 0;

    deflate_init(&d, out, png->size, data + 2, idat - 2);

    if (deflate_decompress(&d) != DEFLATE_OK || d.ooffset != png->size || 2 + d.ioffset + 4 > idat)
        return 0;

    png->data = out;

    if (adler32(png->data, png->size) != read32(data + 2 + d.ioffset))
        return 0;

    return unfilter(png);

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

void png_readrow(struct png *png, unsigned int row, unsigned int *pixels)
{

    unsigned char *line = png->data + row * (png->stride + 1) + 1;
    unsigned int x;

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

}
