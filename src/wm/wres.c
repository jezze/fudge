#include <abi.h>
#include <fudge.h>
#include <math/box.h>
#include <widget/widget.h>
#include "render.h"

static unsigned int quit;

static void oninit(struct event_header *iheader, struct event_header *oheader)
{

    event_request(oheader, iheader, EVENT_WMMAP, EVENT_ADDR_BROADCAST);
    event_send(oheader);

}

static void ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datastop *datastop = event_getdata(iheader);

    event_reply(oheader, iheader, EVENT_DATASTOP);
    event_adddatastop(oheader, datastop->session);
    event_send(oheader);

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_request(oheader, iheader, EVENT_WMUNMAP, EVENT_ADDR_BROADCAST);
    event_send(oheader);

    quit = 1;

}

static void onwmmousepress(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmmousepress *wmmousepress = event_getdata(iheader);

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

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_INIT:
            oninit(iheader, oheader);

            break;

        case EVENT_DATASTOP:
            ondatastop(iheader, oheader);

            break;

        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        case EVENT_WMMOUSEPRESS:
            onwmmousepress(iheader, oheader);

            break;

        }

    }

    event_close();

}

