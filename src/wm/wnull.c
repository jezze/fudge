#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"

static unsigned char backbuffer[4096];
static struct ctrl_videosettings settings;

static unsigned int colormap4[] = {
    0x00000000,
    0x00FFFFFF,
    0x00181014,
    0x0020181C,
    0x0030282C,
    0x00105070,
    0x00307090,
    0x00B05070,
    0x00F898B8
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
        b[i] = colormap4[color];

}

void fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (settings.bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(color, offset, count);

        break;

    }

}

static void flush(unsigned int line, unsigned int offset, unsigned int count)
{

    unsigned int bpp = settings.bpp / 8;

    video_draw(line * bpp + offset * bpp, count * bpp, backbuffer + offset * bpp);

}

static void draw(struct box *bb)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        flush(settings.w * line, bb->x, bb->w);

    }

    video_close();

}

static void sendevent(unsigned int size, unsigned int count, void *buffer)
{

    call_walk(CALL_L2, CALL_PR, 17, "system/event/send");
    call_open(CALL_L2);
    call_write(CALL_L2, 0, size, count, buffer);
    call_close(CALL_L2);

}

static void sendwmmap()
{

    struct event_wmmap wmmap;

    wmmap.header.source = 0;
    wmmap.header.destination = 0xFFFFFFFF;
    wmmap.header.type = EVENT_WMMAP;
    wmmap.header.count = sizeof (struct event_wmmap) - sizeof (struct event_header);

    sendevent(sizeof (struct event_wmmap), 1, &wmmap);

}

static void pollevent()
{

    union event event;
    unsigned int count, roff, quit = 0;
    struct box size;

    size.x = 0;
    size.y = 0;
    size.w = 0;
    size.h = 0;

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &event.header)); roff += count)
    {

        if (event.header.count)
            count += call_read(CALL_L1, roff + count, event.header.count, 1, event.data + sizeof (struct event_header));

        switch (event.header.type)
        {

        case EVENT_WMRESIZE:
            {

            size.x = event.wmresize.x;
            size.y = event.wmresize.y;
            size.w = event.wmresize.w;
            size.h = event.wmresize.h;

            }

            break;

        case EVENT_WMDRAW:
            {

            struct box bb;
            unsigned int w;
            unsigned int h;

            if (event.wmdraw.x >= size.x + size.w)
                break;

            if (event.wmdraw.y >= size.y + size.h)
                break;

            if (event.wmdraw.x + event.wmdraw.w < size.x)
                break;

            if (event.wmdraw.y + event.wmdraw.h < size.y)
                break;

            if (event.wmdraw.x > size.x)
                w = (size.x + size.w) - event.wmdraw.x;
            else
                w = size.w;

            if (event.wmdraw.y > size.y)
                h = (size.y + size.h) - event.wmdraw.y;
            else
                h = size.h;

            bb.x = (event.wmdraw.x < size.x) ? size.x : event.wmdraw.x;
            bb.y = (event.wmdraw.y < size.y) ? size.y : event.wmdraw.y;
            bb.w = (event.wmdraw.x + event.wmdraw.w > size.x + size.w) ? w : event.wmdraw.w;
            bb.h = (event.wmdraw.y + event.wmdraw.h > size.y + size.h) ? h : event.wmdraw.h;

            draw(&bb);

            }

            break;

        case EVENT_WMQUIT:
            quit = 1;

            break;

        }

        if (quit)
            break;

    }

    call_close(CALL_L1);

}

void main()
{

    video_getmode(&settings);
    fill(0x02, 0, 4096);
    sendwmmap();
    pollevent();

}

