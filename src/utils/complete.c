#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void complete(struct event_header *iheader, struct event_header *oheader, unsigned int descriptor, void *name, unsigned int length, unsigned int session)
{

    struct record record;

    file_open(descriptor);
    event_reply(oheader, iheader, EVENT_DATA);
    event_adddata(oheader, session);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            if (event_avail(oheader) < record.length + 1)
            {

                event_send(oheader);
                event_reply(oheader, iheader, EVENT_DATA);
                event_adddata(oheader, session);

            }

            event_appenddata(oheader, record.length, record.name);
            event_appenddata(oheader, 1, "\n");

        }

        if (!file_step(descriptor))
            break;

    }

    event_send(oheader);
    file_close(descriptor);

}

static void onkill(struct event_header *iheader, struct event_header *oheader)
{

    event_reply(oheader, iheader, EVENT_EXIT);
    event_send(oheader);

    quit = 1;

}

static void ondata(struct event_header *iheader, struct event_header *oheader)
{

    struct event_data *data = event_getdata(iheader);

    complete(iheader, oheader, FILE_PW, data + 1, data->count, data->session);

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

        }

    }

    event_close();

}

