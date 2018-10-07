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

static void ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    event_reply(oheader, iheader, EVENT_DATAPIPE);
    event_adddatapipe(oheader, datapipe->session);
    event_appenddata(oheader, datapipe->count, datapipe + 1);
    event_send(oheader);

}

static void ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!datafile->descriptor)
        return;

    file_open(datafile->descriptor);

    while ((count = file_read(datafile->descriptor, buffer, FUDGE_BSIZE - sizeof (struct event_header) - sizeof (struct event_datapipe))))
    {

        event_reply(oheader, iheader, EVENT_DATAPIPE);
        event_adddatapipe(oheader, datafile->session);
        event_appenddata(oheader, count, buffer);
        event_send(oheader);

    }

    file_close(datafile->descriptor);

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

        case EVENT_DATAPIPE:
            ondatapipe(iheader, oheader);

            break;

        case EVENT_DATAFILE:
            ondatafile(iheader, oheader);

            break;

        }

    }

    event_close();

}

