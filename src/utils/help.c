#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addresponse(message, header, EVENT_CHILD);
    event_send(message);

    quit = 1;

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_getdata(header);
    char message[FUDGE_BSIZE];
    unsigned int id;

    if (file->descriptor)
        return;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_CI, "/data/help.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    event_addrequest(message, header, EVENT_INIT, id);
    event_send(message);
    event_addpipe(message, header, EVENT_FILE, id);
    event_addfile(message, FILE_PI);
    event_send(message);
    event_addrequest(message, header, EVENT_EXIT, id);
    event_send(message);

}

void main(void)
{

    event_open();

    while (!quit)
    {

        char data[FUDGE_BSIZE];
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_FILE:
            onfile(header);

            break;

        }

    }

    event_close();

}

