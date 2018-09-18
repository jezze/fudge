#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void oninit(struct event_header *header)
{

    char buffer[FUDGE_BSIZE];
    unsigned int id;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_CI, "/data/motd.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    file_open(FILE_L0);
    event_addheader(buffer, EVENT_INIT, header->target, id);
    event_addforward(buffer, header->source);
    event_sendbuffer(FILE_L0, buffer);
    event_addheader(buffer, EVENT_FILE, header->target, id);
    event_addforward(buffer, header->source);
    event_addfile(buffer, FILE_PI);
    event_sendbuffer(FILE_L0, buffer);
    event_addheader(buffer, EVENT_EXIT, header->target, id);
    event_sendbuffer(FILE_L0, buffer);
    file_close(FILE_L0);

}

static void onkill(struct event_header *header)
{

    event_reply(FILE_L0, header, EVENT_CHILD);

    quit = 1;

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

        case EVENT_INIT:
            oninit(header);

            break;

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header);

            break;

        }

    }

    file_close(FILE_L0);

}

