#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void dump(struct event_header *header, unsigned int count, void *buffer)
{

    char *data = buffer;
    unsigned int i;

    for (i = 0; i < count; i++)
    {

        char message[FUDGE_BSIZE];
        unsigned char num[FUDGE_NSIZE];
        unsigned int cnum = ascii_wzerovalue(num, FUDGE_NSIZE, data[i], 16, 2, 0);

        event_addresponse(message, header, EVENT_DATA);
        event_adddata(message, cnum, num);
        event_sendbuffer(message);
        event_addresponse(message, header, EVENT_DATA);
        event_adddata(message, 2, "  ");
        event_sendbuffer(message);

    }

}

static void ondata(struct event_header *header)
{

    struct event_data *data = event_getdata(header);

    dump(header, data->count, data + 1);

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_getdata(header);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->num);

    while ((count = file_read(file->num, buffer, FUDGE_BSIZE)))
        dump(header, count, buffer);

    file_close(file->num);

}

static void onkill(struct event_header *header)
{

    char message[FUDGE_BSIZE];

    event_addresponse(message, header, EVENT_CHILD);
    event_sendbuffer(message);

    quit = 1;

}

void main(void)
{

    /* Once event system is inside kernel, there is no need to use the filesystem to send events */
    file_walk(FILE_PM, "/system/event");
    file_open(FILE_PM);

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

    file_close(FILE_PM);

}

