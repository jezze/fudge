#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "draw.h"
#include "send.h"

static struct ctrl_videosettings settings;

static unsigned int colormap4[256] = {
    0xFF000000,
    0xFFFFFFFF,
    0xFF181014,
    0xFF20181C,
    0xFF30282C,
    0xFF105070,
    0xFF307090,
    0xFFB05070,
    0xFFF898B8
};

void fill(unsigned int color, unsigned int offset, unsigned int count)
{

    switch (settings.bpp)
    {

    case 8:
        fill8(color, offset, count);

        break;

    case 32:
        fill32(colormap4[color], offset, count);

        break;

    }

}

static void draw(struct box *bb)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        flush(settings.w * line, settings.bpp, bb->x, bb->w);

    }

    video_close();

}

static void pollevent()
{

    union event event;
    unsigned int count, roff, quit = 0;
    struct box size;

    box_setsize(&size, 0, 0, 0, 0);
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
    colormap4[0xFF] = 0x00FF00FF;
    send_wmmap(0xFFFFFFFF);
    pollevent();

}

