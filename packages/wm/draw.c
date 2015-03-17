#include <abi.h>
#include <fudge.h>
#include "box.h"
#include "draw.h"

static unsigned char colormap[] = {
    0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF,
    0x02, 0x02, 0x02,
    0x04, 0x04, 0x04,
    0x06, 0x06, 0x06,
    0x08, 0x10, 0x18,
    0x0C, 0x14, 0x1C
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
    call_write(CALL_L0, 0, 21, colormap);
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

static void draw_buffer(unsigned int offset, unsigned int count, unsigned char *buffer)
{

    call_write(CALL_L0, offset * SCREEN_BPP, count * SCREEN_BPP, buffer);

}

static unsigned char backbuffer[4096];

void backbuffer_drawrepeated(struct box *box)
{

    unsigned int i;

    for (i = 0; i < box->h; i++)
        draw_buffer(box->y * SCREEN_WIDTH + box->x + SCREEN_WIDTH * i, box->w, backbuffer);

}

void backbuffer_fill(unsigned int color, unsigned int count)
{

    unsigned int i;

    for (i = 0; i < count; i++)
        memory_copy(&backbuffer[i * SCREEN_BPP], &color, SCREEN_BPP);

}

void backbuffer_fillbox(struct box *box, unsigned int color)
{

    backbuffer_fill(color, box->w);
    backbuffer_drawrepeated(box);

}

