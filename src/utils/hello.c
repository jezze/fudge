#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void onkill(struct event_header *header, void *message)
{

    event_reply(message, header, EVENT_EXIT);
    event_send(message);

    quit = 1;

}

static void onfile(struct event_header *header, void *message)
{

    struct event_file *file = event_getdata(header);

    if (file->descriptor)
        return;

    event_reply(message, header, EVENT_DATA);
    event_adddata(message, file->session);
    event_appenddata(message, 13, "Hello world!\n");
    event_send(message);

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        char message[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header, message);

            break;

        case EVENT_FILE:
            onfile(header, message);

            break;

        }

    }

    event_close();

}

