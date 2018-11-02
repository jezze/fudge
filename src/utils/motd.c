#include <abi.h>
#include <fudge.h>

static unsigned int onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
    unsigned int id;

    if (file->descriptor)
        return 0;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return 0;

    if (!file_walk2(FILE_C0, "/data/motd.txt"))
        return 0;

    id = call_spawn();

    if (!id)
        return 0;

    event_forwardinit(oheader, iheader, id);
    event_send(oheader);
    event_forwardfile(oheader, iheader, id, file->session, FILE_P0);
    event_send(oheader);
    event_forwardstop(oheader, iheader, id, file->session);
    event_send(oheader);

    return 0;

}

static unsigned int onstop(struct event_header *iheader, struct event_header *oheader)
{

    struct event_stop *stop = event_getdata(iheader);

    event_replystop(oheader, iheader, stop->session);
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

        case EVENT_FILE:
            status = onfile(iheader, oheader);

            break;

        case EVENT_STOP:
            status = onstop(iheader, oheader);

            break;

        case EVENT_KILL:
            status = onkill(iheader, oheader);

            break;

        }

    }

    event_close();

}

