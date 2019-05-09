#include <fudge.h>
#include <abi.h>

static unsigned int (*signals[EVENTS])(struct event_channel *channel);

static void complete(struct event_channel *channel, unsigned int descriptor, void *name, unsigned int length)
{

    struct record record;

    file_open(descriptor);
    event_reply(channel, EVENT_DATA);

    while (file_readall(descriptor, &record, sizeof (struct record)))
    {

        if (record.length >= length && memory_match(record.name, name, length))
        {

            if (event_avail(&channel->o) < record.length + 1)
            {

                event_place(channel->o.header.target, channel);
                event_reply(channel, EVENT_DATA);

            }

            event_append(&channel->o, record.length, record.name);
            event_append(&channel->o, 1, "\n");

        }

        if (!file_step(descriptor))
            break;

    }

    event_place(channel->o.header.target, channel);
    file_close(descriptor);

}

static unsigned int ondata(struct event_channel *channel)
{

    complete(channel, FILE_PW, event_getdata(channel), event_getdatasize(channel));

    return 0;

}

static unsigned int onstop(struct event_channel *channel)
{

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    event_initsignals(signals);

    signals[EVENT_DATA] = ondata;
    signals[EVENT_STOP] = onstop;

    while (!status)
    {

        unsigned int type = event_pick(&channel);

        status = signals[type](&channel);

    }

}

