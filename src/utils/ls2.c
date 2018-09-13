#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void onrein(struct event_header *header, void *data)
{

    char num = *(char *)data;
    struct record record;

    file_open(FILE_PO);
    file_open(FILE_PI + num);
    file_writeall(FILE_PO, "..\n", 3);

    while (file_readall(FILE_PI + num, &record, sizeof (struct record)))
    {

        file_writeall(FILE_PO, record.name, record.length);
        file_writeall(FILE_PO, "\n", 1);
        file_step(FILE_PI + num);

    }

    file_close(FILE_PI + num);
    file_close(FILE_PO);

}

static void oninit(struct event_header *header, void *data)
{

    struct record record;

    file_open(FILE_PO);
    file_open(FILE_PW);
    file_writeall(FILE_PO, "..\n", 3);

    while (file_readall(FILE_PW, &record, sizeof (struct record)))
    {

        file_writeall(FILE_PO, record.name, record.length);
        file_writeall(FILE_PO, "\n", 1);
        file_step(FILE_PW);

    }

    file_close(FILE_PW);
    file_close(FILE_PO);

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

        case EVENT_INIT:
            oninit(&event.header, event.data);

            break;

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(&event.header, event.data);

            break;

        case EVENT_REIN:
            onrein(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L0);

}

