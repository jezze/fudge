#include <fudge.h>
#include "gif.h"

#define NOTRANSPARENT                   0xFFFFFFFF
#define NOCODE                          0xFFFF

struct bits
{

    unsigned int value;
    unsigned int count;
    unsigned int ended;

};

static unsigned int nextbyte(struct gif *gif, unsigned int *byte)
{

    if (gif->position >= gif->count)
    {

        gif->count = gif->read(gif, gif->input, GIF_INPUT, gif->offset);
        gif->offset += gif->count;
        gif->position = 0;

        if (!gif->count)
            return 0;

    }

    byte[0] = gif->input[gif->position++];

    return 1;

}

static unsigned int readbytes(struct gif *gif, unsigned char *buffer, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned int byte;

        if (!nextbyte(gif, &byte))
            return 0;

        buffer[i] = byte;

    }

    return 1;

}

static unsigned int readpalette(struct gif *gif, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned char rgb[3];

        if (!readbytes(gif, rgb, 3))
            return 0;

        gif->palette[i] = 0xFF000000 | ((unsigned int)rgb[0] << 16) | ((unsigned int)rgb[1] << 8) | rgb[2];

    }

    return 1;

}

static unsigned int skipblocks(struct gif *gif)
{

    unsigned int size;

    while (nextbyte(gif, &size) && size)
    {

        unsigned int byte;

        while (size--)
        {

            if (!nextbyte(gif, &byte))
                return 0;

        }

    }

    return 1;

}

unsigned int gif_open(struct gif *gif, unsigned int (*read)(struct gif *gif, void *buffer, unsigned int count, unsigned int offset))
{

    unsigned char header[13];
    unsigned int label;

    buffer_clear(gif->palette, sizeof (gif->palette));

    gif->read = read;
    gif->offset = 0;
    gif->position = 0;
    gif->count = 0;
    gif->transparent = NOTRANSPARENT;

    if (!readbytes(gif, header, 13) || !buffer_match(header, "GIF8", 4))
        return 0;

    gif->width = header[6] | (header[7] << 8);
    gif->height = header[8] | (header[9] << 8);

    if ((header[10] & 0x80) && !readpalette(gif, 2 << (header[10] & 7)))
        return 0;

    /* extensions come before the first image, the graphic control one says which color is transparent */
    while (nextbyte(gif, &label))
    {

        if (label == 0x21)
        {

            unsigned int kind;
            unsigned char control[5];

            if (!nextbyte(gif, &kind))
                return 0;

            if (kind == 0xF9)
            {

                if (!readbytes(gif, control, 5))
                    return 0;

                if (control[1] & 1)
                    gif->transparent = control[4];

            }

            if (!skipblocks(gif))
                return 0;

        }

        else if (label == 0x2C)
        {

            unsigned char descriptor[9];

            if (!readbytes(gif, descriptor, 9))
                return 0;

            gif->left = descriptor[0] | (descriptor[1] << 8);
            gif->top = descriptor[2] | (descriptor[3] << 8);
            gif->framewidth = descriptor[4] | (descriptor[5] << 8);
            gif->frameheight = descriptor[6] | (descriptor[7] << 8);
            gif->interlaced = descriptor[8] & 0x40;

            if ((descriptor[8] & 0x80) && !readpalette(gif, 2 << (descriptor[8] & 7)))
                return 0;

            return gif->width && gif->height;

        }

        else
        {

            return 0;

        }

    }

    return 0;

}

static unsigned int readcode(struct gif *gif, struct bits *bits, unsigned int size)
{

    unsigned int code;

    /* the codes are packed low bits first into sub-blocks of at most 255 bytes */
    while (bits->count < size)
    {

        unsigned int byte;

        if (!gif->blockleft && (!nextbyte(gif, &gif->blockleft) || !gif->blockleft))
        {

            bits->ended = 1;

            return 0;

        }

        if (!nextbyte(gif, &byte))
        {

            bits->ended = 1;

            return 0;

        }

        gif->blockleft--;
        bits->value |= byte << bits->count;
        bits->count += 8;

    }

    code = bits->value & ((1 << size) - 1);
    bits->value >>= size;
    bits->count -= size;

    return code;

}

static unsigned int rowof(struct gif *gif, unsigned int line)
{

    unsigned int h = gif->frameheight;
    unsigned int pass1 = (h + 7) / 8;
    unsigned int pass2 = (h + 3) / 8;
    unsigned int pass3 = (h + 1) / 4;

    /* interlaced rows come as every 8th from 0, every 8th from 4, every 4th from 2, then every 2nd from 1 */
    if (!gif->interlaced)
        return line;

    if (line < pass1)
        return line * 8;

    line -= pass1;

    if (line < pass2)
        return line * 8 + 4;

    line -= pass2;

    if (line < pass3)
        return line * 4 + 2;

    return (line - pass3) * 2 + 1;

}

unsigned int gif_decode(struct gif *gif, unsigned int *pixels)
{

    struct bits bits;
    unsigned int minimum;
    unsigned int clear;
    unsigned int size;
    unsigned int next;
    unsigned int old = NOCODE;
    unsigned int first = 0;
    unsigned int total = gif->framewidth * gif->frameheight;
    unsigned int done = 0;
    unsigned int i;

    buffer_clear(pixels, gif->width * gif->height * 4);

    if (!nextbyte(gif, &minimum) || minimum < 2 || minimum > 8)
        return 0;

    clear = 1 << minimum;
    size = minimum + 1;
    next = clear + 2;
    bits.value = 0;
    bits.count = 0;
    bits.ended = 0;
    gif->blockleft = 0;

    for (i = 0; i < clear; i++)
    {

        gif->prefix[i] = NOCODE;
        gif->suffix[i] = i;

    }

    while (done < total)
    {

        unsigned int code = readcode(gif, &bits, size);
        unsigned int in = code;
        unsigned int depth = 0;

        if (bits.ended || code == clear + 1)
            break;

        if (code == clear)
        {

            size = minimum + 1;
            next = clear + 2;
            old = NOCODE;

            continue;

        }

        if (code > next || (old == NOCODE && code >= clear))
            return 0;

        /* a code not in the table yet is the previous string plus its own first byte */
        if (code == next)
        {

            gif->stack[depth++] = first;
            in = old;

        }

        while (in >= clear && depth < GIF_CODES)
        {

            gif->stack[depth++] = gif->suffix[in];
            in = gif->prefix[in];

        }

        first = in;
        gif->stack[depth++] = first;

        if (old != NOCODE && next < GIF_CODES)
        {

            gif->prefix[next] = old;
            gif->suffix[next] = first;
            next++;

            if (next == (1u << size) && size < 12)
                size++;

        }

        old = code;

        while (depth && done < total)
        {

            unsigned int index = gif->stack[--depth];
            unsigned int x = gif->left + done % gif->framewidth;
            unsigned int y = gif->top + rowof(gif, done / gif->framewidth);

            if (x < gif->width && y < gif->height && index != gif->transparent)
                pixels[y * gif->width + x] = gif->palette[index];

            done++;

        }

    }

    return done > 0;

}

