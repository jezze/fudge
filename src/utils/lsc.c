#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void list(struct event_header *header, unsigned int descriptor, unsigned int session)
{

    char message[FUDGE_BSIZE];
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

        event_addresponse(message, header, EVENT_DATA);
        event_adddata(message, session, count, buffer);
        event_send(message);

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);
    event_addresponse(message, header, EVENT_DATA);
    event_adddata(message, session, 0, 0);
    event_send(message);

}

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

    if (file->descriptor)
        list(header, file->descriptor, file->session);
    else
        list(header, FILE_PW, file->session);

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

