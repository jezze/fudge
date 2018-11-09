#include <fudge.h>
#include <abi.h>

static unsigned int onstop(struct event_header *iheader, struct event_header *oheader)
{

    unsigned int id;

    if (!file_walk2(FILE_CP, "/bin/echo"))
        return 0;

    if (!file_walk2(FILE_C0, "/data/help.txt"))
        return 0;

    id = call_spawn();

    if (!id)
        return 0;

    event_forwardinit(oheader, iheader, id);
    event_send(oheader);
    event_forwardfile(oheader, iheader, id, FILE_P0);
    event_send(oheader);
    event_forwardstop(oheader, iheader, id);
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
    char ibuffer[FUDGE_BSIZE];
    char obuffer[FUDGE_BSIZE];
    struct event_header *oheader = event_init(ibuffer, obuffer);

    event_open();

    while (!status)
    {

        struct event_header *iheader = event_read(ibuffer);

        switch (iheader->type)
        {

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

