#include <abi.h>
#include <fudge.h>
#include <video/video.h>
#include "box.h"
#include "draw.h"
#include "send.h"

static void draw(struct ctrl_videosettings *settings, struct box *bb)
{

    unsigned int line;

    video_open();

    for (line = bb->y; line < bb->y + bb->h; line++)
    {

        draw_flush(settings->w * line, settings->bpp, bb->x, bb->w);

    }

    video_close();

}

static void pollevent(struct ctrl_videosettings *settings, struct box *screen)
{

    union event event;
    unsigned int count, roff, quit = 0;

    call_walk(CALL_L1, CALL_PR, 17, "system/event/poll");
    call_open(CALL_L1);

    for (roff = 0; (count = call_read(CALL_L1, roff, sizeof (struct event_header), 1, &event.header)); roff += count)
    {

        if (event.header.count)
            count += call_read(CALL_L1, roff + count, event.header.count, 1, event.data + sizeof (struct event_header));

        switch (event.header.type)
        {

        case EVENT_WMRESIZE:
            screen->x = event.wmresize.x;
            screen->y = event.wmresize.y;
            screen->w = event.wmresize.w;
            screen->h = event.wmresize.h;

            break;

        case EVENT_WMDRAW:
            {

            struct box bb;
            unsigned int w;
            unsigned int h;

            if (event.wmdraw.x >= screen->x + screen->w)
                break;

            if (event.wmdraw.y >= screen->y + screen->h)
                break;

            if (event.wmdraw.x + event.wmdraw.w < screen->x)
                break;

            if (event.wmdraw.y + event.wmdraw.h < screen->y)
                break;

            if (event.wmdraw.x > screen->x)
                w = (screen->x + screen->w) - event.wmdraw.x;
            else
                w = screen->w;

            if (event.wmdraw.y > screen->y)
                h = (screen->y + screen->h) - event.wmdraw.y;
            else
                h = screen->h;

            bb.x = (event.wmdraw.x < screen->x) ? screen->x : event.wmdraw.x;
            bb.y = (event.wmdraw.y < screen->y) ? screen->y : event.wmdraw.y;
            bb.w = (event.wmdraw.x + event.wmdraw.w > screen->x + screen->w) ? w : event.wmdraw.w;
            bb.h = (event.wmdraw.y + event.wmdraw.h > screen->y + screen->h) ? h : event.wmdraw.h;

            draw(settings, &bb);

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

void main(void)
{

    struct ctrl_videosettings settings;
    struct box screen;

    video_getmode(&settings);
    draw_init();
    box_setsize(&screen, 0, 0, settings.w, settings.h);
    draw_fill(settings.bpp, WM_COLOR_BODY, 0, screen.w);
    send_wmmap(0xFFFFFFFF);
    pollevent(&settings, &screen);

}

