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
    unsigned int id;

    if (file->descriptor)
        return;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_C0, "/data/help.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    event_request(message, header, EVENT_INIT, id);
    event_send(message);
    event_forward(message, header, EVENT_FILE, id);
    event_addfile(message, file->session, FILE_P0);
    event_send(message);
    event_request(message, header, EVENT_EXIT, id);
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

