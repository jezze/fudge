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

        event_senddata(FILE_L0, header->target, header->source, count, buffer);

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

}

static void oninit(struct event_header *header)
{

    list(header, FILE_PW);

}

static void onkill(struct event_header *header)
{

    event_sendchild(FILE_L0, header->target, header->source);

    quit = 1;

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_payload(header);

    list(header, file->num);

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

        case EVENT_FILE:
            onfile(header);

            break;

        }

    }

    file_close(FILE_L0);

}

