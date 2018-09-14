#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void ondata(struct event_header *header, void *data)
{

    unsigned char *buffer = data;
    unsigned int i;

    file_open(FILE_PO);

    for (i = 0; i < header->length - sizeof (struct event_header); i++)
    {

        unsigned char num[FUDGE_NSIZE];

        file_writeall(FILE_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, buffer[i], 16, 2, 0));
        file_writeall(FILE_PO, "  ", 2);

    }

    file_close(FILE_PO);

}

static void onrein(struct event_header *header, void *data)
{

    struct event_rein *rein = data;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(rein->num);
    file_open(FILE_PO);

    while ((count = file_read(rein->num, buffer, FUDGE_BSIZE)))
    {

        unsigned int i;

        for (i = 0; i < count; i++)
        {

            unsigned char num[FUDGE_NSIZE];

            file_writeall(FILE_PO, num, ascii_wzerovalue(num, FUDGE_NSIZE, buffer[i], 16, 2, 0));
            file_writeall(FILE_PO, "  ", 2);

        }

    }

    file_close(FILE_PO);
    file_close(rein->num);

}

static void onkill(struct event_header *header, void *data)
{

    quit = 1;

}

void main(void)
{

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    file_open(FILE_L0);

    while (!quit)
    {

        struct event event;

        event_read(FILE_L0, &event);

        switch (event.header.type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(&event.header, event.data);

            break;

        case EVENT_REIN:
            onrein(&event.header, event.data);

            break;

        case EVENT_DATA:
            ondata(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L0);

}

