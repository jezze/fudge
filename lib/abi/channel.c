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

    switch (redirect->mode)
    {

    case EVENT_REDIRECT_TARGET:
        channel->callbacks[redirect->event].target = redirect->id;

        break;

    case EVENT_REDIRECT_SOURCE:
        channel->callbacks[redirect->event].target = source;

        break;

    default:
        channel->callbacks[redirect->event].target = 0;

        break;

    }

}

static unsigned int sendredirect(struct channel *channel, unsigned int target, unsigned int event, unsigned int mode, unsigned int id)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = mode;
    redirect.id = id;

    return channel_sendbuffer(channel, target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_dispatch(struct channel *channel, struct message_header *header, struct message_data *data)
{

    if (header->event < CHANNEL_CALLBACKS)
    {

        if (channel->callbacks[header->event].callback)
            channel->callbacks[header->event].callback(channel, header->source, data->buffer, message_datasize(header));

    }

}

unsigned int channel_send(struct channel *channel, unsigned int target, unsigned int event)
{

    struct message_header header;

    message_initheader(&header, event, 0);

    while (!call_place(target, &header, 0));

    return 0;

}

unsigned int channel_sendbuffer(struct channel *channel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct message_header header;

    message_initheader(&header, event, count);

    while (!call_place(target, &header, data));

    return count;

}

unsigned int channel_sendstring(struct channel *channel, unsigned int target, unsigned int event, char *string)
{

    struct message_header header;
    struct message_data data;
    unsigned int count = ascii_length(string);

    message_initheader(&header, event, count);
    message_putbuffer(&data, count, string, 0);

    while (!call_place(target, &header, &data));

    return count;

}

unsigned int channel_sendstringz(struct channel *channel, unsigned int target, unsigned int event, char *string)
{

    struct message_header header;
    struct message_data data;
    unsigned int count = ascii_lengthz(string);

    message_initheader(&header, event, count);
    message_putbuffer(&data, count, string, 0);

    while (!call_place(target, &header, &data));

    return count;

}

unsigned int channel_sendredirectsame(struct channel *channel, unsigned int target, unsigned int event)
{

    return sendredirect(channel, target, event, EVENT_REDIRECT_TARGET, channel->callbacks[event].target);

}

unsigned int channel_sendredirectto(struct channel *channel, unsigned int target, unsigned int event, unsigned int id)
{

    return sendredirect(channel, target, event, EVENT_REDIRECT_TARGET, id);

}

unsigned int channel_sendredirectback(struct channel *channel, unsigned int target, unsigned int event)
{

    return sendredirect(channel, target, event, EVENT_REDIRECT_SOURCE, 0);

}

unsigned int channel_reply(struct channel *channel, unsigned int event, unsigned int count, void *data)
{

    return (channel->callbacks[event].target) ? channel_sendbuffer(channel, channel->callbacks[event].target, event, count, data) : 0;

}

unsigned int channel_replystring(struct channel *channel, unsigned int event, char *string)
{

    return channel_reply(channel, event, ascii_length(string), string);

}

unsigned int channel_replystringz(struct channel *channel, unsigned int event, char *string)
{

    return channel_reply(channel, event, ascii_lengthz(string), string);

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

unsigned int channel_polldescriptor(struct channel *channel, unsigned int descriptor, struct message_header *header, struct message_data *data)
{

    return channel_pollsource(channel, 0, header, data);

}

unsigned int channel_polldescriptorevent(struct channel *channel, unsigned int descriptor, unsigned int event, struct message_header *header, struct message_data *data)
{

    return channel_pollsourceevent(channel, 0, event, header, data);

}

unsigned int channel_process(struct channel *channel)
{

    struct message_header header;
    struct message_data data;

    if (channel_poll(channel, &header, &data))
    {

        channel_dispatch(channel, &header, &data);

        return header.event;

    }

    return 0;

}

unsigned int channel_readsource(struct channel *channel, unsigned int source, void *buffer, unsigned int count)
{

    struct message_header header;
    struct message_data data;

    while (channel_poll(channel, &header, &data))
    {

        if (header.source == source)
        {

            switch (header.event)
            {

            case EVENT_CLOSE:
                return 0;

            case EVENT_DATA:
                return buffer_write(buffer, count, data.buffer, message_datasize(&header), 0);

            default:
                channel_dispatch(channel, &header, &data);

                break;

            }

        }

        else
        {

            channel_dispatch(channel, &header, &data);

        }

    }

    return 0;

}

unsigned int channel_readdescriptor(struct channel *channel, unsigned int descriptor, void *buffer, unsigned int count)
{

    struct message_header header;
    struct message_data data;

    while (channel_poll(channel, &header, &data))
    {

        if (header.source == 0)
        {

            switch (header.event)
            {

            case EVENT_CLOSE:
                return 0;

            case EVENT_DATA:
                return buffer_write(buffer, count, data.buffer, message_datasize(&header), 0);

            default:
                channel_dispatch(channel, &header, &data);

                break;

            }

        }

        else
        {

            channel_dispatch(channel, &header, &data);

        }

    }

    return 0;

}

unsigned int channel_wait(struct channel *channel, unsigned int source, unsigned int event)
{

    struct message_header header;
    struct message_data data;

    while (channel_poll(channel, &header, &data))
    {

        if (header.source == source && header.event == event)
            return header.event;

        channel_dispatch(channel, &header, &data);

    }

    return 0;

}

void channel_close(struct channel *channel)
{

    channel->poll = 0;

    channel_reply(channel, EVENT_CLOSE, 0, 0);

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

        channel->callbacks[i].target = 0;
        channel->callbacks[i].callback = 0;

    }

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_TERM, onterm);
    channel_setcallback(channel, EVENT_REDIRECT, onredirect);

}

