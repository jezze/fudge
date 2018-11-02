#include <abi.h>
#include <fudge.h>
#include <widget/widget.h>
#include <widget/render.h>

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    event_requestwmmap(oheader, iheader, EVENT_BROADCAST);
    event_send(oheader);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_requestwmunmap(oheader, iheader, EVENT_BROADCAST);
    event_send(oheader);

    return 1;

}

static unsigned int onwmmousepress(struct event_header *iheader, struct event_header *oheader)
{

    struct event_wmmousepress *wmmousepress = event_getdata(iheader);

    switch (wmmousepress->button)
    {

    case 0x01:
        if (!file_walk2(FILE_L0, "/system/video/if:0/ctrl"))
            break;

        render_init();
        render_setvideo(FILE_L0, 1920, 1080, 4);

        break;

    }

    return 0;

}

void main(void)
{

    unsigned int status = 0;

    event_open();

    while (!status)
    {

        char ibuffer[FUDGE_BSIZE];
        char obuffer[FUDGE_BSIZE];
        struct event_header *iheader = event_read(ibuffer);
        struct event_header *oheader = (struct event_header *)obuffer;

        switch (iheader->type)
        {

        case EVENT_INIT:
            status = oninit(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        case EVENT_WMMOUSEPRESS:
            status = onwmmousepress(iheader, oheader);

            break;

        }

    }

    event_close();

}

