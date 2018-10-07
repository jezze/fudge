#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_DATASTOP);
    event_send(oheader);

    quit = 1;

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    quit = 1;

}

static void ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);

    if (datafile->descriptor)
        return;

    event_reply(oheader, iheader, EVENT_DATAPIPE);
    event_adddatapipe(oheader, datafile->session);
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

        case EVENT_DATASTOP:
            ondatastop(iheader, oheader);

            break;

        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        case EVENT_DATAFILE:
            ondatafile(iheader, oheader);

            break;

        }

    }

    event_close();

}

