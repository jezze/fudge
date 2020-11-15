#include <fudge.h>
#include "call.h"
#include "file.h"
#include "channel.h"

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onterm(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    channel_close(channel);

}

static void onredirect(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_redirect *redirect = mdata;

    channel_setredirect(channel, redirect->type, redirect->mode, redirect->id, source);

}

void channel_dispatch(struct channel *channel, struct message_header *header, struct message_data *data)
{

    if (channel->signals[header->type].callback)
        channel->signals[header->type].callback(channel, header->source, data->buffer, message_datasize(header));

    if (channel->signals[EVENT_ANY].callback)
        channel->signals[EVENT_ANY].callback(channel, header->source, data->buffer, message_datasize(header));

}

unsigned int channel_place(struct channel *channel, unsigned int id, unsigned int type, unsigned int count, void *data)
{

    struct message_header header;

    message_initheader(&header, type, count);

    if (type < EVENTS)
    {

        if (channel->signals[type].redirect)
            id = channel->signals[type].redirect;

        return call_place(id, &header, data);

    }

    return 0;

}

unsigned int channel_poll(struct channel *channel, struct message_header *header, struct message_data *data)
{

    while (channel->poll)
    {

        if (call_pick(header, data))
            return header->type;

    }

    return 0;

}

unsigned int channel_polltype(struct channel *channel, unsigned int type, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        if (header->type == type)
            return header->type;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

unsigned int channel_pollsource(struct channel *channel, unsigned int source, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        if (header->source == source)
            return header->type;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

unsigned int channel_pollsourcetype(struct channel *channel, unsigned int source, unsigned int type, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        if (header->source == source && header->type == type)
            return header->type;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

void channel_close(struct channel *channel)
{

    channel->poll = 0;

    if (channel->signals[EVENT_CLOSE].redirect)
        channel_place(channel, channel->signals[EVENT_CLOSE].redirect, EVENT_CLOSE, 0, 0);

}

void channel_setredirect(struct channel *channel, unsigned int type, unsigned int mode, unsigned int id, unsigned int source)
{

    switch (mode)
    {

    case 1:
        channel->signals[type].redirect = id;

        break;

    case 2:
        channel->signals[type].redirect = source;

        break;

    default:
        channel->signals[type].redirect = 0;

        break;

    }

}

void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, unsigned int source, void *mdata, unsigned int msize))
{

    channel->signals[type].callback = callback;

}

void channel_init(struct channel *channel)
{

    unsigned int i;

    channel->poll = 1;

    for (i = 0; i < EVENTS; i++)
    {

        channel_setsignal(channel, i, 0);
        channel_setredirect(channel, i, 0, 0, 0);

    }

    channel_setsignal(channel, EVENT_MAIN, onmain);
    channel_setsignal(channel, EVENT_TERM, onterm);
    channel_setsignal(channel, EVENT_REDIRECT, onredirect);

}

