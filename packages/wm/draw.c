#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "video.h"
#include "draw.h"

static unsigned int colormap4[] = {
    0x000000,
    0xFFFFFF,
    0x181014,
    0x20181C,
    0x30282C,
    0x105070,
    0x307090,
    0xB05070,
    0xF898B8
};

static unsigned char backbuffer[4096];

void draw_flush(unsigned int line, unsigned int offset, unsigned int count)
{

    video_draw(line * SCREEN_BPP + offset * SCREEN_BPP, count * SCREEN_BPP, backbuffer + offset * SCREEN_BPP);

}

static void draw_fill1(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int i;

    for (i = offset; i < count + offset; i++)
        backbuffer[i] = (color >> 0) & 0xFF;

}

static void draw_fill4(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int i;

    for (i = offset * SCREEN_BPP; i < count * SCREEN_BPP + offset * SCREEN_BPP; i += SCREEN_BPP)
    {

        backbuffer[i + 0] = (colormap4[color] >> 0) & 0xFF;
        backbuffer[i + 1] = (colormap4[color] >> 8) & 0xFF;
        backbuffer[i + 2] = (colormap4[color] >> 16) & 0xFF;
        backbuffer[i + 3] = (colormap4[color] >> 24) & 0xFF;

    }

}

void draw_fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (SCREEN_BPP)
    {

    case 1:
        draw_fill1(color, offset, count);

        break;

    case 4:
        draw_fill4(color, offset, count);

        break;

    }

}

