#include <fudge.h>
#include <abi.h>

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

                event_place(channel->o.header.target, &channel->o);
                event_reply(channel, EVENT_DATA);

            }

            event_append(&channel->o, record.length, record.name);
            event_append(&channel->o, 1, "\n");

        }

        if (!file_step(descriptor))
            break;

    }

    event_place(channel->o.header.target, &channel->o);
    file_close(descriptor);

}

static unsigned int ondata(struct event_channel *channel)
{

    complete(channel, FILE_PW, event_getdata(channel), event_getdatasize(channel));

    return 0;

}

void main(void)
{

    event_initsignals();
    event_setsignal(EVENT_DATA, ondata);

    while (event_listen());

}

