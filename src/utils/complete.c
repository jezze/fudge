#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void complete(struct event_header *header, void *message, unsigned int descriptor, void *name, unsigned int length, unsigned int session)
{

    struct record record;
    unsigned int n = 0;

    file_open(descriptor);
    event_addresponse(message, header, EVENT_DATA);
    event_adddata(message, session);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            event_appenddata(message, record.length, record.name);
            event_appenddata(message, 1, "\n");

            if (++n == 10)
                break;

        }

        if (!file_step(descriptor))
            break;

    }

    event_send(message);
    file_close(descriptor);

}

static void onkill(struct event_header *header, void *message)
{

    event_addresponse(message, header, EVENT_EXIT);
    event_send(message);

    quit = 1;

}

static void ondata(struct event_header *header, void *message)
{

    struct event_data *data = event_getdata(header);

    complete(header, message, FILE_PW, data + 1, data->count, data->session);

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

        case EVENT_DATA:
            ondata(header, message);

            break;

        }

    }

    event_close();

}

