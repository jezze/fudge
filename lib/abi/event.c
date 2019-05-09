#include <fudge.h>
#include "call.h"
#include "event.h"

static unsigned int nosignal(struct event_channel *channel)
{

    return 0;

}

static unsigned int onkill(struct event_channel *channel)
{

    return 1;

}

unsigned int event_pick(struct event_channel *channel)
{

    while (!call_pick(&channel->i.header, sizeof (struct event_header)));

    if (channel->i.header.length > sizeof (struct event_header))
        while (!call_pick(&channel->i.header + 1, channel->i.header.length - sizeof (struct event_header)));

    return channel->i.header.type;

}

unsigned int event_place(unsigned int id, struct event_channel *channel)
{

    while (!call_place(id, &channel->o, channel->o.header.length));

    return channel->o.header.type;

}

void event_initsignals(unsigned int (*signals[EVENTS])(struct event_channel *channel))
{

    unsigned int i;

    for (i = 0; i < EVENTS; i++)
        signals[i] = nosignal;

    signals[EVENT_KILL] = onkill;

}

