#include <fudge.h>
#include <abi.h>
#include <widget/widget.h>
#include <widget/render.h>

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    event_requestwmmap(oheader, iheader, EVENT_BROADCAST, 0);
    event_send(oheader);

    return 0;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_requestwmunmap(oheader, iheader, EVENT_BROADCAST, 0);
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
    union event_message imessage;
    union event_message omessage;

    event_open();

    while (!status)
    {

        event_read(&imessage);

        switch (imessage.header.type)
        {

        case EVENT_INIT:
            status = oninit(&imessage.header, &omessage.header);

            break;

        case EVENT_KILL:
            status = onkill(&imessage.header, &omessage.header);

            break;

        case EVENT_WMMOUSEPRESS:
            status = onwmmousepress(&imessage.header, &omessage.header);

            break;

        }

    }

    event_close();

}

