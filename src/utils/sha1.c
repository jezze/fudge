#include <abi.h>
#include <fudge.h>

static struct sha1 s;

static unsigned int ondatafile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datafile *datafile = event_getdata(iheader);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!datafile->descriptor)
        return 0;

    file_open(datafile->descriptor);

    while ((count = file_read(datafile->descriptor, buffer, FUDGE_BSIZE)))
        sha1_read(&s, buffer, count);

    file_close(datafile->descriptor);

    return 0;

}

static unsigned int ondatapipe(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datapipe *datapipe = event_getdata(iheader);

    sha1_read(&s, datapipe + 1, datapipe->count);

    return 0;

}

static unsigned int ondatastop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_datastop *datastop = event_getdata(iheader);
    unsigned char digest[20];
    char num[FUDGE_NSIZE];
    unsigned int i;

    sha1_write(&s, digest);
    event_replydatapipe(oheader, iheader, datastop->session);

    for (i = 0; i < 20; i++)
        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, digest[i], 16, 2, 0), num);

    event_appenddata(oheader, 1, "\n");
    event_send(oheader);
    event_replydatastop(oheader, iheader, datastop->session);
    event_send(oheader);

    return 1;

}

static unsigned int oninit(struct event_header *iheader, struct event_header *oheader)
{

    sha1_init(&s);

    return 0;

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

        case EVENT_INIT:
            status = oninit(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();

}

