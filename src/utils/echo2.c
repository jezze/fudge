#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void onrein(struct event_header *header, void *data)
{

    if (!file_walk(FILE_PI, data))
        return;

}

static void oninit(struct event_header *header, void *data)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(FILE_PO);
    file_open(FILE_PI);

    while ((count = file_read(FILE_PI, buffer, FUDGE_BSIZE)))
        file_writeall(FILE_PO, buffer, count);

    file_close(FILE_PI);
    file_close(FILE_PO);

}

static void onexit(struct event_header *header, void *data)
{

    quit = 1;

}

void main(void)
{

    if (!file_walk(FILE_L0, "/system/event"))
        return;

    if (!file_walk(FILE_L1, "/system/wm/event"))
        return;

    file_open(FILE_L0);
    file_open(FILE_L1);

    while (!quit)
    {

        struct event event;

        event_read(FILE_L0, &event);

        switch (event.header.type)
        {

        case EVENT_INIT:
            oninit(&event.header, event.data);

            break;

        case EVENT_EXIT:
            onexit(&event.header, event.data);

            break;

        case EVENT_REIN:
            onrein(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L1);
    file_close(FILE_L0);

}

