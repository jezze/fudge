#include <fudge.h>
#include "call.h"
#include "channel.h"

static void ignore(struct channel *channel, void *mdata, unsigned int msize)
{

}

static void abort(struct channel *channel, void *mdata, unsigned int msize)
{

    channel->state = 0;

}

void channel_place(struct channel *channel, unsigned int id)
{

    while (!call_place(id, &channel->o, channel->data));

}

void channel_listen(struct channel *channel)
{

    while (channel->state)
    {

        char data[FUDGE_BSIZE];
        unsigned int size;

        while (!call_pick(&channel->i, sizeof (struct event_header)));

        size = channel->i.length - sizeof (struct event_header);

        if (size)
            while (!call_pick(data, size));

        channel->signals[channel->i.type](channel, data, size);
        channel->signals[EVENT_ANY](channel, data, size);

    }

}

void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, void *mdata, unsigned int msize))
{

    channel->signals[type] = callback;

}

void channel_forward(struct channel *channel, unsigned int type)
{

    unsigned int i;

    event_create(&channel->o, type, 0);

    channel->o.session = channel->i.session;

    for (i = 0; i < channel->i.nroutes; i++)
        event_addroute(&channel->o, channel->i.routes[i].target, channel->i.routes[i].session);

}

void channel_request(struct channel *channel, unsigned int type)
{

    channel_forward(channel, type);
    event_addroute(&channel->o, channel->i.target, 0);

}

void channel_request2(struct channel *channel, unsigned int type, unsigned int session)
{

    channel_forward(channel, type);

    channel->o.session = session;

    event_addroute(&channel->o, channel->i.target, session);

}

unsigned int channel_reply(struct channel *channel, unsigned int type)
{

    channel_forward(channel, type);

    if (channel->o.nroutes)
    {

        channel->o.nroutes--;
        channel->o.session = channel->o.routes[channel->o.nroutes].session;

        return channel->o.routes[channel->o.nroutes].target;

    }

    return channel->i.source;

}

void channel_append(struct channel *channel, unsigned int count, void *buffer)
{

    channel->o.length += memory_write(&channel->data, FUDGE_BSIZE, buffer, count, channel->o.length - sizeof (struct event_header));

}

void channel_init(struct channel *channel)
{

    unsigned int i;

    channel->state = 1;

    for (i = 0; i < EVENTS; i++)
        channel_setsignal(channel, i, ignore);

    channel_setsignal(channel, EVENT_KILL, abort);
    channel_setsignal(channel, EVENT_STOP, abort);
    event_create(&channel->i, 0, 0);
    event_create(&channel->o, 0, 0);

}

