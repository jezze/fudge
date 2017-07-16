#include <abi.h>
#include <fudge.h>
#include <event/event.h>
#include "box.h"
#include "widget.h"
#include "render.h"

static unsigned int quit;
static char outputdata[FUDGE_BSIZE];
static struct ring output;
static struct event_handlers handlers;

static void onmousepress(struct event_header *header, struct event_mousepress *mousepress)
{

    switch (mousepress->button)
    {

    case 0x01:
        render_init();
        render_setvideo(FILE_L6, 1920, 1080, 32);

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

    handlers.mousepress = onmousepress;
    handlers.wmexit = onwmexit;
    handlers.wmresize = onwmresize;
    handlers.wmshow = onwmshow;
    handlers.wmhide = onwmhide;

    ring_init(&output, FUDGE_BSIZE, outputdata);

    if (!file_walk(FILE_L0, "/system/wm/data"))
        return;

    if (!file_walk(FILE_L1, "/system/wm/event"))
        return;

    if (!file_walk(FILE_L6, "/system/video/if:0/ctrl"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);
    event_sendwmmap(FILE_L1, EVENT_ADDR_BROADCAST);

    while (!quit && event_read(&handlers, FILE_L1))
    {

        if (ring_count(&output))
        {

            file_writeall(FILE_L0, outputdata, ring_count(&output));
            ring_reset(&output);
            event_sendwmflush(FILE_L1, EVENT_ADDR_BROADCAST);

        }

    }

    file_close(FILE_L1);
    file_close(FILE_L0);

}

