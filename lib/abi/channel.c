#include <fudge.h>
#include "call.h"
#include "channel.h"

static void ignore(struct channel *channel)
{

}

static void abort(struct channel *channel)
{

    channel->state = 0;

}

void *channel_getdata(struct channel *channel)
{

    return &channel->i.header + 1;

}

unsigned int channel_getdatasize(struct channel *channel)
{

    return channel->i.header.length - sizeof (struct event_header);

}

unsigned int channel_pick(struct channel *channel)
{

    if (!channel->state)
        return 0;

    while (!call_pick(&channel->i.header, sizeof (struct event_header)));

    if (channel->i.header.length > sizeof (struct event_header))
        while (!call_pick(&channel->i.header + 1, channel->i.header.length - sizeof (struct event_header)));

    return channel->i.header.type;

}

unsigned int channel_place(unsigned int id, union event_message *message)
{

    while (!call_place(id, message, message->header.length));

    return message->header.type;

}

void channel_dispatch(struct channel *channel, unsigned int type)
{

    channel->signals[type](channel);
    channel->signals[EVENT_ANY](channel);

}

unsigned int channel_listen(struct channel *channel)
{

    unsigned int type;

    while ((type = channel_pick(channel)))
        channel_dispatch(channel, type);

    return 0;

}

void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel))
{

    channel->signals[type] = callback;

}

void channel_forward(struct channel *channel, unsigned int type)
{

    unsigned int i;

    event_create(&channel->o, type);

    channel->o.header.session = channel->i.header.session;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

}

void channel_request(struct channel *channel, unsigned int type, unsigned int session)
{

    unsigned int i;

    event_create(&channel->o, type);

    channel->o.header.session = session;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

    event_addroute(&channel->o, channel->i.header.target, session);

}

void channel_reply(struct channel *channel, unsigned int type)
{

    unsigned int i;

    event_create(&channel->o, type);

    channel->o.header.session = channel->i.header.session;
    channel->o.header.target = channel->i.header.source;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

    if (channel->o.header.nroutes)
    {

        channel->o.header.nroutes--;
        channel->o.header.target = channel->o.header.routes[channel->o.header.nroutes].target;
        channel->o.header.session = channel->o.header.routes[channel->o.header.nroutes].session;

    }

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

