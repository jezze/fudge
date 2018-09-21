#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void complete(struct event_header *header, unsigned int descriptor, void *name, unsigned int length, unsigned int stream)
{

    char message[FUDGE_BSIZE];
    char buffer[FUDGE_BSIZE];
    unsigned int count = 0;
    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            count += memory_write(buffer, FUDGE_BSIZE, record.name, record.length, count);
            count += memory_write(buffer, FUDGE_BSIZE, "\n", 1, count);

        }

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);
    event_addresponse(message, header, EVENT_DATA);
    event_adddata(message, stream, count, buffer);
    event_send(message);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addresponse(message, header, EVENT_CHILD);
    event_send(message);

    quit = 1;

}

static void ondata(struct event_header *header)
{

    struct event_data *data = event_getdata(header);

    complete(header, FILE_PW, data + 1, data->count, data->stream);

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

        case EVENT_DATA:
            ondata(header);

            break;

        }

    }

    event_close();

}

