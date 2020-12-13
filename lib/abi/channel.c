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

    channel_setredirect(channel, redirect->event, redirect->mode, redirect->id, source);

}

void channel_dispatch(struct channel *channel, struct message_header *header, struct message_data *data)
{

    if (header->event < CHANNEL_CALLBACKS)
    {

        if (channel->callbacks[header->event].callback)
            channel->callbacks[header->event].callback(channel, header->source, data->buffer, message_datasize(header));

    }

}

unsigned int channel_placefor(struct channel *channel, unsigned int id, unsigned int event, unsigned int count, void *data)
{

    struct message_header header;

    message_initheader(&header, event, count);

    while (!call_place(id, &header, data));

    return count;

}

unsigned int channel_place(struct channel *channel, unsigned int event, unsigned int count, void *data)
{

    return (channel->callbacks[event].target) ? channel_placefor(channel, channel->callbacks[event].target, event, count, data) : 0;

}

unsigned int channel_poll(struct channel *channel, struct message_header *header, struct message_data *data)
{

    while (channel->poll)
    {

        if (call_pick(header, data))
            return header->event;

    }

    return 0;

}

unsigned int channel_pollevent(struct channel *channel, unsigned int event, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        if (header->event == event)
            return header->event;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

unsigned int channel_polldescriptor(struct channel *channel, unsigned int descriptor, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        /* All kernel messages are 0 right now */
        if (header->source == 0)
            return header->event;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

unsigned int channel_polldescriptorevent(struct channel *channel, unsigned int descriptor, unsigned int event, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        /* All kernel messages are 0 right now */
        if (header->source == 0 && header->event == event)
            return header->event;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

unsigned int channel_pollsource(struct channel *channel, unsigned int source, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        if (header->source == source)
            return header->event;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

unsigned int channel_pollsourceevent(struct channel *channel, unsigned int source, unsigned int event, struct message_header *header, struct message_data *data)
{

    while (channel_poll(channel, header, data))
    {

        if (header->source == source && header->event == event)
            return header->event;

        channel_dispatch(channel, header, data);

    }

    return 0;

}

void channel_close(struct channel *channel)
{

    channel->poll = 0;

    channel_place(channel, EVENT_CLOSE, 0, 0);

}

void channel_setredirect(struct channel *channel, unsigned int event, unsigned int mode, unsigned int id, unsigned int source)
{

    switch (mode)
    {

    case EVENT_REDIRECT_TARGET:
        channel->callbacks[event].target = id;

        break;

    case EVENT_REDIRECT_SOURCE:
        channel->callbacks[event].target = source;

        break;

    default:
        channel->callbacks[event].target = 0;

        break;

    }

}

void channel_redirectsame(struct channel *channel, unsigned int target, unsigned int event)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = EVENT_REDIRECT_TARGET;
    redirect.id = channel->callbacks[event].target;

    channel_placefor(channel, target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_redirectto(struct channel *channel, unsigned int target, unsigned int event, unsigned int id)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = EVENT_REDIRECT_TARGET;
    redirect.id = id;

    channel_placefor(channel, target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_redirectback(struct channel *channel, unsigned int target, unsigned int event)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = EVENT_REDIRECT_SOURCE;
    redirect.id = 0;

    channel_placefor(channel, target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_setcallback(struct channel *channel, unsigned int event, void (*callback)(struct channel *channel, unsigned int source, void *mdata, unsigned int msize))
{

    channel->callbacks[event].callback = callback;

}

void channel_init(struct channel *channel, struct channel_callback *callbacks)
{

    unsigned int i;

    channel->poll = 1;
    channel->callbacks = callbacks;

    for (i = 0; i < CHANNEL_CALLBACKS; i++)
    {

        channel_setcallback(channel, i, 0);
        channel_setredirect(channel, i, 0, 0, 0);

    }

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_TERM, onterm);
    channel_setcallback(channel, EVENT_REDIRECT, onredirect);

}

