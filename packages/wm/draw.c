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

static void draw_fill1(void *bb, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned char *b = bb;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = color;

}

static void draw_fill4(void *bb, unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int *b = bb;
    unsigned int i;

    for (i = offset; i < count + offset; i++)
        b[i] = colormap4[color];

}

void draw_fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (SCREEN_BPP)
    {

    case 1:
        draw_fill1(backbuffer, color, offset, count);

        break;

    case 4:
        draw_fill4(backbuffer, color, offset, count);

        break;

    }

}

