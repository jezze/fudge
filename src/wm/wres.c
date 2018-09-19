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

static void oninit(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    ring_init(&output, FUDGE_BSIZE, outputdata);

    event_addrequest(message, header, EVENT_WMMAP, EVENT_ADDR_BROADCAST);
    event_sendbuffer(FILE_L0, message);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addrequest(message, header, EVENT_WMUNMAP, EVENT_ADDR_BROADCAST);
    event_sendbuffer(FILE_L0, message);
    event_addreply(message, header, EVENT_CHILD);
    event_sendbuffer(FILE_L0, message);

    quit = 1;

}

static void onwmmousepress(struct event_header *header)
{

    struct event_wmmousepress *wmmousepress = event_getdata(header);

    switch (wmmousepress->button)
    {

    case 0x01:
        render_init();
        render_setvideo(FILE_L1, 1920, 1080, 4);

        quit = 1;

        break;

    }

}

void main(void)
{

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/video/if:0/ctrl"))
        return;

    file_open(FILE_L0);

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(FILE_L0, data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header);

            break;

        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_WMMOUSEPRESS:
            onwmmousepress(header);

            break;

        }

        if (ring_count(&output))
        {

            event_sendwmflush(FILE_L0, EVENT_ADDR_SELF, EVENT_ADDR_BROADCAST, ring_count(&output), outputdata);
            ring_reset(&output);

        }

    }

    file_close(FILE_L0);

}

