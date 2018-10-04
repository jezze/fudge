#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_EXIT);
    event_send(oheader);

    quit = 1;

}

static void onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
    unsigned int id;

    if (file->descriptor)
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
    event_forward(oheader, iheader, EVENT_FILE, id);
    event_addfile(oheader, file->session, FILE_P0);
    event_send(oheader);
    event_request(oheader, iheader, EVENT_EXIT, id);
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

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(iheader, oheader);

            break;

        case EVENT_FILE:
            onfile(iheader, oheader);

            break;

        }

    }

    event_close();

}

