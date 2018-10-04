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

        event_reply(oheader, iheader, EVENT_DATA);
        event_adddata(oheader, session);
        event_appenddata(oheader, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0), num);
        event_appenddata(oheader, 2, "  ");
        event_send(oheader);

    }

}

static void ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    dump(iheader, oheader, data->count, data + 1, data->session);

}

static void onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        dump(iheader, oheader, count, buffer, file->session);

    file_close(file->descriptor);

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_EXIT);
    event_send(oheader);

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

