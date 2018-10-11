#include <abi.h>
#include <fudge.h>

static unsigned int ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!datafile->descriptor)
        return 0;

    file_open(datafile->descriptor);

    while ((count = file_read(datafile->descriptor, buffer, FUDGE_BSIZE - sizeof (struct event_header) - sizeof (struct event_datapipe))))
    {

        event_replydatapipe(oheader, iheader, datafile->session);
        event_appenddata(oheader, count, buffer);
        event_send(oheader);

    }

    file_close(datafile->descriptor);

    return 0;

}

static unsigned int ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    event_replydatapipe(oheader, iheader, datapipe->session);
    event_appenddata(oheader, datapipe->count, datapipe + 1);
    event_send(oheader);

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

        case EVENT_DATAPIPE:
            status = ondatapipe(iheader, oheader);

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

