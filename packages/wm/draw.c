#include <abi.h>
#include <fudge.h>
#include "box.h"
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

static unsigned char colormap[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x03, 0x02, 0x02,
    0x05, 0x04, 0x04,
    0x07, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C,
    0x28, 0x10, 0x18,
    0x38, 0x20, 0x28
};

void draw_setmode()
{

    call_walk(CALL_L0, CALL_PR, 19, "system/video:0/ctrl");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 0, 0);
    call_close(CALL_L0);

}

void draw_setcolormap()
{

    call_walk(CALL_L0, CALL_PR, 23, "system/video:0/colormap");
    call_open(CALL_L0);
    call_write(CALL_L0, 0, 27, colormap);
    call_close(CALL_L0);

}

void draw_begin()
{

    call_walk(CALL_L0, CALL_PR, 19, "system/video:0/data");
    call_open(CALL_L0);

}

void draw_end()
{

    call_close(CALL_L0);

}

static unsigned char backbuffer[4096];

void backbuffer_drawline(unsigned int line, unsigned int count)
{

    call_write(CALL_L0, line * count, count, backbuffer);

}

static void backbuffer_fill1(unsigned int color, unsigned int offset, unsigned int count)
{

    unsigned int i;

    for (i = offset; i < count + offset; i++)
        backbuffer[i] = (color >> 0) & 0xFF;

}

static void backbuffer_fill4(unsigned int color, unsigned int offset, unsigned int count)
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

static void backbuffer_fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (SCREEN_BPP)
    {

    case 1:
        backbuffer_fill1(color, offset, count);

        break;

    case 4:
        backbuffer_fill4(color, offset, count);

        break;

    }

}

void backbuffer_fillbox(struct box *box, unsigned int color, unsigned int line)
{

    if (line < box->y || line >= box->y + box->h)
        return;

    backbuffer_fill(color, box->x, box->w);

}

void backbuffer_fillcount(unsigned int color, unsigned int offset, unsigned int count)
{

    backbuffer_fill(color, offset, count);

}

