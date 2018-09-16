#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void list(struct event_header *header, unsigned int descriptor)
{

    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char buffer[FUDGE_BSIZE];
        unsigned int count = 0;

        count += ascii_wzerovalue(buffer, FUDGE_BSIZE, record.id, 16, 8, count);
        count += memory_write(buffer, FUDGE_BSIZE, " ", 1, count);
        count += ascii_wzerovalue(buffer, FUDGE_BSIZE, record.size, 16, 8, count);
        count += memory_write(buffer, FUDGE_BSIZE, " ", 1, count);
        count += memory_write(buffer, FUDGE_BSIZE, record.name, record.length, count);
        count += memory_write(buffer, FUDGE_BSIZE, "\n", 1, count);

        event_senddata(FILE_L0, header->destination, header->source, count, buffer);
        file_step(descriptor);

    }

    file_close(descriptor);

}

static void oninit(struct event_header *header, void *data)
{

    list(header, FILE_PW);

}

static void onkill(struct event_header *header, void *data)
{

    event_sendchild(FILE_L0, header->destination, header->source);

    quit = 1;

}

static void onfile(struct event_header *header, void *data)
{

    struct event_file *file = data;

    list(header, file->num);

}

void main(void)
{

    /* Once event system is inside kernel, there is no need to use the filesystem to send events */
    file_walk(FILE_L0, "/system/event");
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

        case EVENT_FILE:
            onfile(&event.header, event.data);

            break;

        }

    }

    file_close(FILE_L0);

}

