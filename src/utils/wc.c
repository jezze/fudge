#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void sum(struct event_header *header, unsigned int count, void *buffer, unsigned int session)
{

    char message[FUDGE_BSIZE];
    char num[FUDGE_NSIZE];
    char *data = buffer;
    unsigned int bytes = 0, words = 0, lines = 0;
    unsigned int whitespace = 1;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        switch (data[i])
        {

        case '\n':
            whitespace = 1;
            lines++;

            break;

        case ' ':
            whitespace = 1;

            break;

        default:
            if (whitespace)
                words++;

            whitespace = 0;

            break;

        }

        bytes++;

    }

    event_addresponse(message, header, EVENT_DATA);
    event_adddata(message, session);
    event_appenddata(message, ascii_wvalue(num, FUDGE_BSIZE, lines, 10), num);
    event_appenddata(message, 1, "\n");
    event_appenddata(message, ascii_wvalue(num, FUDGE_BSIZE, words, 10), num);
    event_appenddata(message, 1, "\n");
    event_appenddata(message, ascii_wvalue(num, FUDGE_BSIZE, bytes, 10), num);
    event_appenddata(message, 1, "\n");
    event_send(message);

}

static void ondata(struct event_header *header)
{

    struct event_data *data = event_getdata(header);

    sum(header, data->count, data + 1, data->session);

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_getdata(header);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    if (!file->descriptor)
        return;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        sum(header, count, buffer, file->session);

    file_close(file->descriptor);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addresponse(message, header, EVENT_CHILD);
    event_send(message);

    quit = 1;

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

        case EVENT_FILE:
            onfile(header);

            break;

        }

    }

    event_close();

}

