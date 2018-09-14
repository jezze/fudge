#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void onkill(struct event_header *header, void *data)
{

    quit = 1;

}

static void ondata(struct event_header *header, void *data)
{

    event_senddata(FILE_L0, header->destination, header->source, header->length - sizeof (struct event_header), data);

}

static void onfile(struct event_header *header, void *data)
{

    struct event_file *file = data;
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->num);

    while ((count = file_read(file->num, buffer, FUDGE_BSIZE)))
        event_senddata(FILE_L0, header->destination, header->source, count, buffer);

    file_close(file->num);

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

        case EVENT_DATA:
            ondata(&event.header, event.data);

            break;

        case EVENT_FILE:
            onfile(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L0);

}

