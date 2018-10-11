#include <abi.h>
#include <fudge.h>

static void list(struct event_header *iheader, struct event_header *oheader, unsigned int descriptor, unsigned int session)
{

    struct record record;

    file_open(descriptor);
    event_replydatapipe(oheader, iheader, session);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (event_avail(oheader) < record.length + 1)
        {

            event_send(oheader);
            event_replydatapipe(oheader, iheader, session);

        }

        event_appenddata(oheader, record.length, record.name);
        event_appenddata(oheader, 1, "\n");

        if (!file_step(descriptor))
            break;

    }

    event_send(oheader);
    file_close(descriptor);

}

static unsigned int ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);

    if (datafile->descriptor)
        list(iheader, oheader, datafile->descriptor, datafile->session);
    else
        list(iheader, oheader, FILE_PW, datafile->session);

    return 0;

}

static unsigned int ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datastop *datastop = event_getdata(iheader);

    event_replydatastop(oheader, iheader, datastop->session);
    event_send(oheader);

    return 1;

}

static unsigned int onkill(struct event_header *iheader, struct event_header *oheader)
{

    return 1;

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

        case EVENT_DATAFILE:
            status = ondatafile(iheader, oheader);

            break;

        case EVENT_DATASTOP:
            status = ondatastop(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();

}

