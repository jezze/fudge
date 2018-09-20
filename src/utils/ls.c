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

        char message[FUDGE_BSIZE];
        char buffer[FUDGE_BSIZE];
        unsigned int count = 0;

        count += memory_write(buffer, FUDGE_BSIZE, record.name, record.length, count);
        count += memory_write(buffer, FUDGE_BSIZE, "\n", 1, count);

        event_addresponse(message, header, EVENT_DATA);
        event_adddata(message, count, buffer);
        event_sendbuffer(message);

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addresponse(message, header, EVENT_CHILD);
    event_sendbuffer(message);

    quit = 1;

}

static void ondata(struct event_header *header)
{

    list(header, FILE_PW);

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_getdata(header);

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
        struct event_header *header = event_read(data);

        switch (header->type)
        {

        case EVENT_EXIT:
        case EVENT_KILL:
            onkill(header);

            break;

        case EVENT_DATA:
            ondata(header);

            break;

        case EVENT_FILE:
            onfile(header);

            break;

        }

    }

    file_close(FILE_L0);

}

