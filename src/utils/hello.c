#include <fudge.h>
#include <abi.h>

static unsigned int onstop(struct event_channel *channel)
{

    event_reply(channel, EVENT_DATA);
    event_append(&channel->o, 13, "Hello world!\n");
    event_place(channel->o.header.target, &channel->o);

    return 1;

}

void main(void)
{

    struct event_channel channel;

    event_initsignals(&channel);
    event_setsignal(&channel, EVENT_STOP, onstop);
    event_listen(&channel);

}

