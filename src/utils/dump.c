#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void dump(struct event_header *iheader, struct event_header *oheader, unsigned int count, void *buffer, unsigned int session)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned char num[FUDGE_NSIZE];

        event_reply(oheader, iheader, EVENT_DATAPIPE);
        event_adddatapipe(oheader, session);
        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0), num);
        event_appenddata(oheader, 2, "  ");
        event_send(oheader);

    }

}

static void ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    dump(iheader, oheader, datapipe->count, datapipe + 1, datapipe->session);

}

static void ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!datafile->descriptor)
        return;

    file_open(datafile->descriptor);

    while ((count = file_read(datafile->descriptor, buffer, FUDGE_BSIZE)))
        dump(iheader, oheader, count, buffer, datafile->session);

    file_close(datafile->descriptor);

}

static void ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datastop *datastop = event_getdata(iheader);

    event_reply(oheader, iheader, EVENT_DATASTOP);
    event_adddatastop(oheader, datastop->session);
    event_send(oheader);

    quit = 1;

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    quit = 1;

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

