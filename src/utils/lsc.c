#include <abi.h>
#include <fudge.h>

static unsigned int quit;

static void list(struct event_header *header, unsigned int descriptor, unsigned int session)
{

    struct record record;

    file_open(descriptor);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        char message[FUDGE_BSIZE];
        char num[FUDGE_BSIZE];

        event_addresponse(message, header, EVENT_DATA);
        event_adddata(message, session);
        event_appenddata(message, ascii_wzerovalue(num, FUDGE_BSIZE, record.id, 16, 8, 0), num);
        event_appenddata(message, 1, " ");
        event_appenddata(message, ascii_wzerovalue(num, FUDGE_BSIZE, record.size, 16, 8, 0), num);
        event_appenddata(message, 1, " ");
        event_appenddata(message, record.length, record.name);
        event_appenddata(message, 1, "\n");
        event_send(message);

        if (!file_step(descriptor))
            break;

    }

    file_close(descriptor);

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

