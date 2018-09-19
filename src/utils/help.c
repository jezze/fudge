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

    char buffer[FUDGE_BSIZE];
    unsigned int id;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_CI, "/data/help.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    file_open(FILE_L0);
    event_addrequest(buffer, header, EVENT_INIT, id);
    event_sendbuffer(FILE_L0, buffer);
    event_addpipe(buffer, header, EVENT_FILE, id);
    event_addfile(buffer, FILE_PI);
    event_sendbuffer(FILE_L0, buffer);
    event_addrequest(buffer, header, EVENT_EXIT, id);
    event_sendbuffer(FILE_L0, buffer);
    file_close(FILE_L0);

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

