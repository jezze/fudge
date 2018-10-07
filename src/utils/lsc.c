#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void list(struct event_header *iheader, struct event_header *oheader, unsigned int descriptor, unsigned int session)
{

    struct record record;

    file_open(descriptor);
    event_reply(oheader, iheader, EVENT_DATAPIPE);
    event_adddatapipe(oheader, session);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char num[FUDGE_NSIZE];

        if (event_avail(oheader) < record.length + 3 + 16)
        {

            event_send(oheader);
            event_reply(oheader, iheader, EVENT_DATAPIPE);
            event_adddatapipe(oheader, session);

        }

        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, record.id, 16, 8, 0), num);
        event_appenddata(oheader, 1, " ");
        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, record.size, 16, 8, 0), num);
        event_appenddata(oheader, 1, " ");
        event_appenddata(oheader, record.length, record.name);
        event_appenddata(oheader, 1, "\n");

        if (!file_step(descriptor))
            break;

    }

    event_send(oheader);
    file_close(descriptor);

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

static void ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);

    if (datafile->descriptor)
        list(iheader, oheader, datafile->descriptor, datafile->session);
    else
        list(iheader, oheader, FILE_PW, datafile->session);

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

