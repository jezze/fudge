#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void list(struct event_header *iheader, struct event_header *oheader, unsigned int descriptor, unsigned int session)
{

    struct record record;

    file_open(descriptor);
    event_reply(oheader, iheader, EVENT_DATA);
    event_adddata(oheader, session);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char num[FUDGE_NSIZE];

        if (event_avail(oheader) < record.length + 3 + 16)
        {

            event_send(oheader);
            event_reply(oheader, iheader, EVENT_DATA);
            event_adddata(oheader, session);

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

static void onexit(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_EXIT);
    event_send(oheader);

    quit = 1;

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    quit = 1;

}

static void onfile(struct event_header *iheader, struct event_header *oheader)
{

    struct event_file *file = event_getdata(iheader);

    if (file->descriptor)
        list(iheader, oheader, file->descriptor, file->session);
    else
        list(iheader, oheader, FILE_PW, file->session);

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
            onexit(iheader, oheader);

            break;

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

