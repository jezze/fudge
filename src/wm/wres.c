#include <abi.h>
#include <fudge.h>
#include <event/event.h>
#include <math/box.h>
#include <widget/widget.h>
#include "render.h"

static unsigned int quit;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct event_handlers handlers;

static void onwmmousepress(struct event_header *header, struct event_wmmousepress *wmmousepress)
{

    switch (wmmousepress->button)
    {

    case 0x01:
        render_init();
        render_setvideo(FILE_L6, 1920, 1080, 32);

        quit = 1;

        break;

    }

}

static void onwmexit(struct event_header *header)
{

    quit = 1;

}

static void onwmresize(struct event_header *header, struct event_wmresize *wmresize)
{

}

static void onwmshow(struct event_header *header)
{

}

static void onwmhide(struct event_header *header)
{

}

void main(void)
{

    handlers.wmmousepress = onwmmousepress;
    handlers.wmexit = onwmexit;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    ring_init(&output, FUDGE_BSIZE, outputdata);

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/wm/data"))
        return;

    if (!file_walk(FILE_L2, "/system/wm/event"))
        return;

    if (!file_walk(FILE_L6, "/system/video/if:0/ctrl"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);
    file_open(FILE_L2);
    event_sendwmmap(FILE_L2, EVENT_ADDR_BROADCAST);

    while (!quit)
    {

        event_read(&handlers, FILE_L0);

        if (ring_count(&output))
        {

            file_writeall(FILE_L1, outputdata, ring_count(&output));
            ring_reset(&output);
            event_sendwmflush(FILE_L2, EVENT_ADDR_BROADCAST);

        }

    }

    event_sendwmunmap(FILE_L2, EVENT_ADDR_BROADCAST);
    file_close(FILE_L2);
    file_close(FILE_L1);
    file_close(FILE_L0);

}

