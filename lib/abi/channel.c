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

unsigned int channel_pick(struct channel *channel)
{

    while (!call_pick(&channel->i.header, sizeof (struct event_header)));

    if (channel->i.header.length > sizeof (struct event_header))
        while (!call_pick(&channel->i.header + 1, channel->i.header.length - sizeof (struct event_header)));

    return channel->i.header.type;

}

unsigned int channel_place(struct channel *channel, unsigned int id)
{

    while (!call_place(id, &channel->o.header, channel->o.header.length));

    return channel->o.header.type;

}

void channel_dispatch(struct channel *channel, unsigned int type)
{

    channel->signals[type](channel, &channel->i.header + 1, channel->i.header.length - sizeof (struct event_header));
    channel->signals[EVENT_ANY](channel, &channel->i.header + 1, channel->i.header.length - sizeof (struct event_header));

}

unsigned int channel_listen(struct channel *channel)
{

    while (channel->state)
        channel_dispatch(channel, channel_pick(channel));

    return 0;

}

void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, void *mdata, unsigned int msize))
{

    channel->signals[type] = callback;

}

void channel_forward(struct channel *channel, unsigned int type)
{

    unsigned int i;

    event_create(&channel->o.header, type, 0);

    channel->o.header.session = channel->i.header.session;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o.header, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

}

void channel_request(struct channel *channel, unsigned int type, unsigned int session)
{

    unsigned int i;

    event_create(&channel->o.header, type, 0);

    channel->o.header.session = session;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o.header, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

    event_addroute(&channel->o.header, channel->i.header.target, session);

}

unsigned int channel_reply(struct channel *channel, unsigned int type)
{

    unsigned int i;

    event_create(&channel->o.header, type, 0);

    channel->o.header.session = channel->i.header.session;
    channel->o.header.target = channel->i.header.source;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o.header, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

    if (channel->o.header.nroutes)
    {

        channel->o.header.nroutes--;
        channel->o.header.target = channel->o.header.routes[channel->o.header.nroutes].target;
        channel->o.header.session = channel->o.header.routes[channel->o.header.nroutes].session;

    }

    return channel->o.header.target;

}

void channel_append(struct channel *channel, unsigned int count, void *buffer)
{

    channel->o.header.length += memory_write(&channel->o.header, FUDGE_BSIZE, buffer, count, channel->o.header.length);

}

void channel_init(struct channel *channel)
{

    unsigned int i;

    channel->state = 1;

    for (i = 0; i < EVENTS; i++)
        channel_setsignal(channel, i, ignore);

    channel_setsignal(channel, EVENT_KILL, abort);
    channel_setsignal(channel, EVENT_STOP, abort);

}

