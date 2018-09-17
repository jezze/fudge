#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void oninit(struct event_header *header)
{

    unsigned int id;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_CI, "/data/help.txt"))
        return;

    id = call_spawn();

    if (!id)
        return;

    file_open(FILE_L0);
    event_sendinit(FILE_L0, header->source, id);
    event_sendfile(FILE_L0, header->source, id, FILE_PI);
    event_sendexit(FILE_L0, header->source, id);
    file_close(FILE_L0);

}

static void onkill(struct event_header *header)
{

    event_sendchild(FILE_L0, header->target, header->source);

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

