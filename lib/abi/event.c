#include <fudge.h>
#include "call.h"
#include "event.h"

static unsigned int ignore(struct event_channel *channel)
{

    return 0;

}

static unsigned int abort(struct event_channel *channel)
{

    return 1;

}

unsigned int event_place(unsigned int id, union event_message *message)
{

    while (!call_place(id, message, message->header.length));

    return message->header.type;

}

unsigned int event_listen(struct event_channel *channel)
{

    while (!call_pick(&channel->i.header, sizeof (struct event_header)));

    if (channel->i.header.length > sizeof (struct event_header))
        while (!call_pick(&channel->i.header + 1, channel->i.header.length - sizeof (struct event_header)));

    if (channel->signals[channel->i.header.type])
    {

        if (channel->signals[channel->i.header.type](channel))
            call_despawn();

        return channel->i.header.type;

    }

    return 0;

}

void event_setsignal(struct event_channel *channel, unsigned int type, unsigned int (*callback)(struct event_channel *channel))
{

    channel->signals[type] = callback;

}

void event_clearsignal(struct event_channel *channel, unsigned int type)
{

    channel->signals[type] = 0;

}

void event_initsignals(struct event_channel *channel)
{

    unsigned int i;

    for (i = 0; i < EVENTS; i++)
        event_setsignal(channel, i, ignore);

    event_setsignal(channel, EVENT_KILL, abort);
    event_setsignal(channel, EVENT_STOP, abort);

}

