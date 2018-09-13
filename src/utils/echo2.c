#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void ondata(struct event_header *header, void *data)
{

    file_open(FILE_PO);
    file_writeall(FILE_PO, data, header->length - sizeof (struct event_header));
    file_close(FILE_PO);

}

static void onrein(struct event_header *header, void *data)
{

    char num = *(char *)data;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(FILE_PI + num);

    while ((count = file_read(FILE_PI + num, buffer, FUDGE_BSIZE)))
    {

        file_open(FILE_PO);
        file_writeall(FILE_PO, buffer, count);
        file_close(FILE_PO);

    }

    file_close(FILE_PI + num);

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

