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

static unsigned int redirect(unsigned int target, unsigned int event, unsigned int mode, unsigned int id)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = mode;
    redirect.id = id;

    return send(target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_dispatch(struct message *message)
{

    if (message->header.event < CHANNEL_CALLBACKS)
    {

        if (callbacks[message->header.event].callback)
            callbacks[message->header.event].callback(message->header.source, message->data.buffer, message_datasize(&message->header));

    }

}

unsigned int channel_send(unsigned int event)
{

    return (callbacks[event].target) ? send(callbacks[event].target, event, 0, 0) : 0;

}

unsigned int channel_sendto(unsigned int target, unsigned int event)
{

    return send(target, event, 0, 0);

}

unsigned int channel_sendbuffer(unsigned int event, unsigned int count, void *data)
{

    return (callbacks[event].target) ? send(callbacks[event].target, event, count, data) : 0;

}

unsigned int channel_sendbufferto(unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return send(target, event, count, data);

}

unsigned int channel_sendstring(unsigned int event, char *string)
{

    return (callbacks[event].target) ? send(callbacks[event].target, event, ascii_length(string), string) : 0;

}

unsigned int channel_sendstringto(unsigned int target, unsigned int event, char *string)
{

    return send(target, event, ascii_length(string), string);

}

unsigned int channel_sendstringz(unsigned int event, char *string)
{

    return (callbacks[event].target) ? send(callbacks[event].target, event, ascii_lengthz(string), string) : 0;

}

unsigned int channel_sendstringzto(unsigned int target, unsigned int event, char *string)
{

    return send(target, event, ascii_lengthz(string), string);

}

unsigned int channel_sendmessage(struct message *message)
{

    return (callbacks[message->header.event].target) ? send(callbacks[message->header.event].target, message->header.event, message_datasize(&message->header), message->data.buffer) : 0;

}

unsigned int channel_sendmessageto(unsigned int target, struct message *message)
{

    return send(target, message->header.event, message_datasize(&message->header), message->data.buffer);

}

unsigned int channel_redirectsame(unsigned int target, unsigned int event)
{

    return redirect(target, event, EVENT_REDIRECT_TARGET, callbacks[event].target);

}

unsigned int channel_redirecttarget(unsigned int target, unsigned int event, unsigned int id)
{

    return redirect(target, event, EVENT_REDIRECT_TARGET, id);

}

unsigned int channel_redirectback(unsigned int target, unsigned int event)
{

    return redirect(target, event, EVENT_REDIRECT_SOURCE, 0);

}

unsigned int channel_poll(struct message *message)
{

    while (poll)
    {

        if (call_pick(&message->header, &message->data))
            return message->header.event;

    }

    return 0;

}

unsigned int channel_pollsource(unsigned int source, struct message *message)
{

    while (channel_poll(message))
    {

        if (message->header.source == source)
            return message->header.event;

        channel_dispatch(message);

    }

    return 0;

}

unsigned int channel_pollevent(unsigned int event, struct message *message)
{

    while (channel_poll(message))
    {

        if (message->header.event == event)
            return message->header.event;

        channel_dispatch(message);

    }

    return 0;

}

unsigned int channel_pollsourceevent(unsigned int source, unsigned int event, struct message *message)
{

    while (channel_poll(message))
    {

        if (message->header.source == source && message->header.event == event)
            return message->header.event;

        channel_dispatch(message);

    }

    return 0;

}

unsigned int channel_polldescriptor(unsigned int descriptor, struct message *message)
{

    return channel_pollsource(0, message);

}

unsigned int channel_polldescriptorevent(unsigned int descriptor, unsigned int event, struct message *message)
{

    return channel_pollsourceevent(0, event, message);

}

unsigned int channel_process(void)
{

    struct message message;

    if (channel_poll(&message))
    {

        channel_dispatch(&message);

        return message.header.event;

    }

    return 0;

}

unsigned int channel_readsource(unsigned int source, void *buffer, unsigned int count)
{

    struct message message;

    while (channel_poll(&message))
    {

        if (message.header.source == source)
        {

            switch (message.header.event)
            {

            case EVENT_CLOSE:
                return 0;

            case EVENT_DATA:
                return buffer_write(buffer, count, message.data.buffer, message_datasize(&message.header), 0);

            default:
                channel_dispatch(&message);

                break;

            }

        }

        else
        {

            channel_dispatch(&message);

        }

    }

    return 0;

}

unsigned int channel_readdescriptor(unsigned int descriptor, void *buffer, unsigned int count)
{

    struct message message;

    while (channel_poll(&message))
    {

        if (message.header.source == 0)
        {

            switch (message.header.event)
            {

            case EVENT_CLOSE:
                return 0;

            case EVENT_DATA:
                return buffer_write(buffer, count, message.data.buffer, message_datasize(&message.header), 0);

            default:
                channel_dispatch(&message);

                break;

            }

        }

        else
        {

            channel_dispatch(&message);

        }

    }

    return 0;

}

unsigned int channel_wait(unsigned int source, unsigned int event)
{

    struct message message;

    while (channel_poll(&message))
    {

        if (message.header.source == source && message.header.event == event)
            return message.header.event;

        channel_dispatch(&message);

    }

    return 0;

}

void channel_close(void)
{

    poll = 0;

    channel_send(EVENT_CLOSE);

}

void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize))
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

