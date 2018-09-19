#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addreply(message, header, EVENT_CHILD);
    event_sendbuffer(FILE_L0, message);

    quit = 1;

}

static void ondata(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addreply(message, header, EVENT_DATA);
    event_adddata(message, 13, "Hello world!\n");
    event_sendbuffer(FILE_L0, message);

}

void main(void)
{

    /* Once event system is inside kernel, there is no need to use the filesystem to send events */
    file_walk(FILE_L0, "/system/event");
    file_open(FILE_L0);

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(FILE_L0, data);

        switch (header->type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_DATA:
            ondata(header);

            break;

        }

    }

    file_close(FILE_L0);

}

