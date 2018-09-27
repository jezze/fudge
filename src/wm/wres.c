#include <abi.h>
#include <fudge.h>
#include <math/box.h>
#include <widget/widget.h>
#include "render.h"

static unsigned int quit;

static void oninit(struct event_header *header, void *message)
{

    event_addrequest(message, header, EVENT_WMMAP, EVENT_ADDR_BROADCAST);
    event_send(message);

}

static void onkill(struct event_header *header, void *message)
{

    event_addrequest(message, header, EVENT_WMUNMAP, EVENT_ADDR_BROADCAST);
    event_send(message);
    event_addresponse(message, header, EVENT_CHILD);
    event_send(message);

    quit = 1;

}

static void onwmmousepress(struct event_header *header, void *message)
{

    struct event_wmmousepress *wmmousepress = event_getdata(header);

    switch (wmmousepress->button)
    {

    case 0x01:
        if (!file_walk(FILE_L0, "/system/video/if:0/ctrl"))
            break;

        render_init();
        render_setvideo(FILE_L0, 1920, 1080, 4);

        break;

    }

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        char message[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_INIT:
            oninit(header, message);

            break;

        case EVENT_KILL:
            onkill(header, message);

            break;

        case EVENT_WMMOUSEPRESS:
            onwmmousepress(header, message);

            break;

        }

    }

    event_close();

}

