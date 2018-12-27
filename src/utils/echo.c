#include <fudge.h>
#include <abi.h>

static unsigned int ondata(struct event_channel *channel)
{

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, event_getdatasize(channel), event_getdata(channel));
    event_place(channel->o.header.target, &channel->o);

    return 0;

}

static unsigned int onfile(struct event_channel *channel)
{

    struct event_file *file = event_getdata(channel);
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE - sizeof (struct event_header))))
    {

        event_reply(channel, EVENT_DATA);
        event_append(&channel->o, count, buffer);
        event_place(channel->o.header.target, &channel->o);

    }

    file_close(file->descriptor);

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

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

