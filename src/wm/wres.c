#include <abi.h>
#include <fudge.h>
#include <event/base.h>
#include <event/wm.h>
#include <math/box.h>
#include <widget/widget.h>
#include "render.h"

static unsigned int quit;
static char outputdata[FUDGE_BSIZE];
static struct ring output;

static void oninit(struct event_header *header, void *data)
{

    event_sendwmmap(FILE_L1, EVENT_ADDR_BROADCAST);

}

static void onkill(struct event_header *header, void *data)
{

    event_sendwmunmap(FILE_L1, EVENT_ADDR_BROADCAST);

    quit = 1;

}

static void onwmmousepress(struct event_header *header, void *data)
{

    struct event_wmmousepress *wmmousepress = data;

    switch (wmmousepress->button)
    {

    case 0x01:
        render_init();
        render_setvideo(FILE_L6, 1920, 1080, 4);

        quit = 1;

        break;

    }

}

void main(void)
{

    ring_init(&output, FUDGE_BSIZE, outputdata);

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/wm/event"))
        return;

    if (!file_walk(FILE_L6, "/system/video/if:0/ctrl"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);

    while (!quit)
    {

        struct event event;

        event_read(FILE_L0, &event);

        switch (event.header.type)
        {

        case EVENT_INIT:
            oninit(&event.header, event.data);

            break;

        case EVENT_KILL:
            onkill(&event.header, event.data);

            break;

        case EVENT_WMMOUSEPRESS:
            onwmmousepress(&event.header, event.data);

            break;

        }

        if (ring_count(&output))
        {

            event_sendwmflush(FILE_L1, EVENT_ADDR_BROADCAST, ring_count(&output), outputdata);
            ring_reset(&output);

        }

    }

    file_close(FILE_L1);
    file_close(FILE_L0);

}

