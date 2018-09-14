#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void oninit(struct event_header *header, void *data)
{

    unsigned int id;

    if (!file_walk(FILE_CP, "/bin/echo"))
        return;

    if (!file_walk(FILE_CI, "/data/motd.txt"))
        return;

    id = call_spawn();

    file_open(FILE_L0);
    event_sendinit(FILE_L0, header->source, id);
    event_sendfile(FILE_L0, header->source, id, FILE_PI);
    event_sendexit(FILE_L0, header->source, id);
    file_close(FILE_L0);

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

        }

    }

    file_close(FILE_L0);

}

