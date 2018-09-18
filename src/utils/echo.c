#include <abi.h>
#include <fudge.h>
#include <event/base.h>

static unsigned int quit;

static void onkill(struct event_header *header)
{

    event_sendchild(FILE_L0, header->target, header->source);

    quit = 1;

}

static void ondata(struct event_header *header)
{

    struct event_data *data = event_getdata(header);

    event_senddata(FILE_L0, header->target, header->source, data->count, data + 1);

}

static void onfile(struct event_header *header)
{

    struct event_file *file = event_getdata(header);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->num);

    while ((count = file_read(file->num, buffer, FUDGE_BSIZE)))
        event_senddata(FILE_L0, header->target, header->source, count, buffer);

    file_close(file->num);

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

