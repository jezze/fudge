#include <fudge.h>
#include "call.h"
#include "event.h"

static void ignore(struct event_channel *channel)
{

}

static void abort(struct event_channel *channel)
{

    channel->state = 0;

}

static unsigned int readmsg(struct event_channel *channel)
{

    while (!call_pick(&channel->i.header, sizeof (struct event_header)));

    if (channel->i.header.length > sizeof (struct event_header))
        while (!call_pick(&channel->i.header + 1, channel->i.header.length - sizeof (struct event_header)));

    return channel->i.header.type;

}

void *event_getdata(struct event_channel *channel)
{

    return &channel->i.header + 1;

}

unsigned int event_getdatasize(struct event_channel *channel)
{

    return channel->i.header.length - sizeof (struct event_header);

}

unsigned int event_place(unsigned int id, union event_message *message)
{

    while (!call_place(id, message, message->header.length));

    return message->header.type;

}

unsigned int event_listen(struct event_channel *channel)
{

    while (channel->state)
    {

        unsigned int type = readmsg(channel);

        if (!channel->signals[type])
            return type;

        channel->signals[type](channel);
        channel->signals[EVENT_ANY](channel);

    }

    return 0;

}

void event_setsignal(struct event_channel *channel, unsigned int type, void (*callback)(struct event_channel *channel))
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

    channel->state = 1;

    for (i = 0; i < EVENTS; i++)
        event_setsignal(channel, i, ignore);

    event_setsignal(channel, EVENT_KILL, abort);
    event_setsignal(channel, EVENT_STOP, abort);

}

void event_forward(struct event_channel *channel, unsigned int type)
{

    unsigned int i;

    event_create(&channel->o, type);

    channel->o.header.session = channel->i.header.session;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

}

void event_request(struct event_channel *channel, unsigned int type, unsigned int session)
{

    unsigned int i;

    event_create(&channel->o, type);

    channel->o.header.session = session;

    for (i = 0; i < channel->i.header.nroutes; i++)
        event_addroute(&channel->o, channel->i.header.routes[i].target, channel->i.header.routes[i].session);

    event_addroute(&channel->o, channel->i.header.target, session);

}

void event_reply(struct event_channel *channel, unsigned int type)
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

