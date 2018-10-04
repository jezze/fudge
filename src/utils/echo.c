#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_EXIT);
    event_send(oheader);

    quit = 1;

}

static void ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    event_reply(oheader, iheader, EVENT_DATA);
    event_adddata(oheader, data->session);
    event_appenddata(oheader, data->count, data + 1);
    event_send(oheader);

}

static void onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE - sizeof (struct event_header) - sizeof (struct event_data))))
    {

        event_reply(oheader, iheader, EVENT_DATA);
        event_adddata(oheader, file->session);
        event_appenddata(oheader, count, buffer);
        event_send(oheader);

    }

    file_close(file->descriptor);

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

        case EVENT_DATA:
            ondata(iheader, oheader);

            break;

        case EVENT_FILE:
            onfile(iheader, oheader);

            break;

        }

    }

    event_close();

}

