#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "draw.h"

static unsigned char backbuffer[0x2000];

static unsigned char colormap[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x03, 0x02, 0x02,
    0x05, 0x04, 0x04,
    0x07, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C,
    0x28, 0x10, 0x18,
    0x38, 0x20, 0x28,
    0x1C, 0x18, 0x18,
    0xFF, 0xFF, 0xFF
};

static unsigned int colormap4[] = {
    0xFF000000,
    0xFFFFFFFF,
    0xFF181014,
    0xFF20181C,
    0xFF30282C,
    0xFF105070,
    0xFF307090,
    0xFFB05070,
    0xFFF898B8,
    0xFF80787C,
    0xFFFFFFFF
};

static void fill8(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *b = (unsigned char *)backbuffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = color;

}

static void fill32(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *b = (unsigned int *)backbuffer;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = color;

}

void draw_fill(unsigned int bpp, unsigned int color, unsigned int offset, unsigned int count)
{

    switch (bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(colormap4[color], offset, count);

        break;

    }

}

void draw_flush(unsigned int line, unsigned int bpp, unsigned int offset, unsigned int count)
{

    video_draw(CALL_L0, line + offset, count, backbuffer + offset * bpp / 8);

}

void draw_init()
{

    video_setcolormap(CALL_L0, 0, 3, 11, colormap);

}

