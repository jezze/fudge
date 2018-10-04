#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_EXIT);
    event_send(oheader);

    quit = 1;

}

static void onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);

    if (file->descriptor)
        return;

    event_reply(oheader, iheader, EVENT_DATA);
    event_adddata(oheader, file->session);
    event_appenddata(oheader, 13, "Hello world!\n");
    event_send(oheader);

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

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        case EVENT_FILE:
            onfile(iheader, oheader);

            break;

        }

    }

    event_close();

}

