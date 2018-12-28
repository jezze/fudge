#include <fudge.h>
#include <abi.h>

static struct crc s;

static unsigned int ondata(struct event_channel *channel)
{

    crc_read(&s, event_getdata(channel), event_getdatasize(channel));

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        crc_read(&s, buffer, count);

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int result = crc_finalize(&s);

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, ascii_wvalue(buffer, 32, result, 10), buffer);
    event_append(&channel->o, 1, "\n");
    event_place(channel->o.header.target, channel);

    return 1;

}

static unsigned int onkill(struct event_channel *channel)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    while (!status)
    {

        switch (event_pick(&channel))
        {

        case EVENT_DATA:
            status = ondata(&channel);

            break;

        case EVENT_FILE:
            status = onfile(&channel);

            break;

        case EVENT_STOP:
            status = onstop(&channel);

            break;

        case EVENT_KILL:
            status = onkill(&channel);

            break;

        }

    }

}

