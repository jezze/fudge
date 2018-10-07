#include <abi.h>
#include <fudge.h>

static unsigned int quit;

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
    unsigned int id;

    if (datafile->descriptor)
        return;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_C0, "/data/motd.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    event_request(oheader, iheader, EVENT_INIT, id);
    event_send(oheader);
    event_forward(oheader, iheader, EVENT_DATAFILE, id);
    event_adddatafile(oheader, datafile->session, FILE_P0);
    event_send(oheader);
    event_request(oheader, iheader, EVENT_DATASTOP, id);
    event_adddatastop(oheader, datafile->session);
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

