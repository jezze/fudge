#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void dump(struct event_header *header, void *message, unsigned int count, void *buffer, unsigned int session)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        unsigned char num[FUDGE_NSIZE];

        event_reply(message, header, EVENT_DATA);
        event_adddata(message, session);
        event_appenddata(message, ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0), num);
        event_appenddata(message, 2, "  ");
        event_send(message);

    }

}

static void ondata(struct event_header *header, void *message)
{

    struct event_data *data = event_getdata(header);

    dump(header, message, data->count, data + 1, data->session);

}

static void onfile(struct event_header *header, void *message)
{

    struct event_file *file = event_getdata(header);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        dump(header, message, count, buffer, file->session);

    file_close(file->descriptor);

}

static void onkill(struct event_header *header, void *message)
{

    event_reply(message, header, EVENT_EXIT);
    event_send(message);

    quit = 1;

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

        case EVENT_FILE:
            onfile(header, message);

            break;

        }

    }

    event_close();

}

