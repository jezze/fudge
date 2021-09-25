#include <fudge.h>
#include "call.h"
#include "file.h"
#include "channel.h"

#define CHANNEL_CALLBACKS               256

static struct channel_callback
{

    void (*callback)(unsigned int source, void *data, unsigned int size);
    unsigned int target;

} callbacks[CHANNEL_CALLBACKS];

static unsigned int poll;

static unsigned int send(unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct message_header header;
    unsigned int offset = 0;
    char *buffer = data;

    while (count > MESSAGE_SIZE)
    {

        message_initheader(&header, event, MESSAGE_SIZE);

        while (!call_place(target, &header, buffer + offset));

        count -= MESSAGE_SIZE;
        offset += MESSAGE_SIZE;

    }

    message_initheader(&header, event, count);

    while (!call_place(target, &header, buffer + offset));

    return count + offset;

}

static unsigned int sendredirect(unsigned int target, unsigned int event, unsigned int mode, unsigned int id)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = mode;
    redirect.id = id;

    return send(target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_dispatch(struct message_header *header, struct message_data *data)
{

    if (header->event < CHANNEL_CALLBACKS)
    {

        if (callbacks[header->event].callback)
            callbacks[header->event].callback(header->source, data->buffer, message_datasize(header));

    }

}

unsigned int channel_send(unsigned int target, unsigned int event)
{

    return send(target, event, 0, 0);

}

unsigned int channel_sendbuffer(unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return send(target, event, count, data);

}

unsigned int channel_sendstring(unsigned int target, unsigned int event, char *string)
{

    return send(target, event, ascii_length(string), string);

}

unsigned int channel_sendstringz(unsigned int target, unsigned int event, char *string)
{

    return send(target, event, ascii_lengthz(string), string);

}

unsigned int channel_sendredirectsame(unsigned int target, unsigned int event)
{

    return sendredirect(target, event, EVENT_REDIRECT_TARGET, callbacks[event].target);

}

unsigned int channel_sendredirectto(unsigned int target, unsigned int event, unsigned int id)
{

    return sendredirect(target, event, EVENT_REDIRECT_TARGET, id);

}

unsigned int channel_sendredirectback(unsigned int target, unsigned int event)
{

    return sendredirect(target, event, EVENT_REDIRECT_SOURCE, 0);

}

unsigned int channel_reply(unsigned int event, unsigned int count, void *data)
{

    return (callbacks[event].target) ? send(callbacks[event].target, event, count, data) : 0;

}

unsigned int channel_replystring(unsigned int event, char *string)
{

    return channel_reply(event, ascii_length(string), string);

}

unsigned int channel_replystringz(unsigned int event, char *string)
{

    return channel_reply(event, ascii_lengthz(string), string);

}

unsigned int channel_poll(struct message_header *header, struct message_data *data)
{

    while (poll)
    {

        if (call_pick(header, data))
            return header->event;

    }

    return 0;

}

unsigned int channel_pollsource(unsigned int source, struct message_header *header, struct message_data *data)
{

    while (channel_poll(header, data))
    {

        if (header->source == source)
            return header->event;

        channel_dispatch(header, data);

    }

    return 0;

}

unsigned int channel_pollevent(unsigned int event, struct message_header *header, struct message_data *data)
{

    while (channel_poll(header, data))
    {

        if (header->event == event)
            return header->event;

        channel_dispatch(header, data);

    }

    return 0;

}

unsigned int channel_pollsourceevent(unsigned int source, unsigned int event, struct message_header *header, struct message_data *data)
{

    while (channel_poll(header, data))
    {

        if (header->source == source && header->event == event)
            return header->event;

        channel_dispatch(header, data);

    }

    return 0;

}

unsigned int channel_polldescriptor(unsigned int descriptor, struct message_header *header, struct message_data *data)
{

    return channel_pollsource(0, header, data);

}

unsigned int channel_polldescriptorevent(unsigned int descriptor, unsigned int event, struct message_header *header, struct message_data *data)
{

    return channel_pollsourceevent(0, event, header, data);

}

unsigned int channel_process(void)
{

    struct message_header header;
    struct message_data data;

    if (channel_poll(&header, &data))
    {

        channel_dispatch(&header, &data);

        return header.event;

    }

    return 0;

}

unsigned int channel_readsource(unsigned int source, void *buffer, unsigned int count)
{

    struct message_header header;
    struct message_data data;

    while (channel_poll(&header, &data))
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
                channel_dispatch(&header, &data);

                break;

            }

        }

        else
        {

            channel_dispatch(&header, &data);

        }

    }

    return 0;

}

unsigned int channel_readdescriptor(unsigned int descriptor, void *buffer, unsigned int count)
{

    struct message_header header;
    struct message_data data;

    while (channel_poll(&header, &data))
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
                channel_dispatch(&header, &data);

                break;

            }

        }

        else
        {

            channel_dispatch(&header, &data);

        }

    }

    return 0;

}

unsigned int channel_wait(unsigned int source, unsigned int event)
{

    struct message_header header;
    struct message_data data;

    while (channel_poll(&header, &data))
    {

        if (header.source == source && header.event == event)
            return header.event;

        channel_dispatch(&header, &data);

    }

    return 0;

}

void channel_close(void)
{

    poll = 0;

    channel_reply(EVENT_CLOSE, 0, 0);

}

void channel_setcallback(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize))
{

    callbacks[event].callback = callback;

}

void channel_redirect(unsigned int event, unsigned int mode, unsigned int id, unsigned int source)
{

    switch (mode)
    {

    case EVENT_REDIRECT_TARGET:
        callbacks[event].target = id;

        break;

    case EVENT_REDIRECT_SOURCE:
        callbacks[event].target = source;

        break;

    default:
        callbacks[event].target = 0;

        break;

    }

}

void channel_init(void)
{

    unsigned int i;

    poll = 1;

    for (i = 0; i < CHANNEL_CALLBACKS; i++)
    {

        callbacks[i].target = 0;
        callbacks[i].callback = 0;

    }

}

