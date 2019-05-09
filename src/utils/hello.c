#include <fudge.h>
#include <abi.h>

static unsigned int (*signals[EVENTS])(struct event_channel *channel);

static unsigned int onstop(struct event_channel *channel)
{

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, 13, "Hello world!\n");
    event_place(channel->o.header.target, channel);

    return 1;

}

void main(void)
{

    unsigned int status = 0;
    struct event_channel channel;

    event_initsignals(signals);

    signals[EVENT_STOP] = onstop;

    while (!status)
    {

        unsigned int type = event_pick(&channel);

        status = signals[type](&channel);

    }

}

