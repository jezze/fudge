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

static unsigned int active;

static unsigned int send(unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct message_header header;
    unsigned int offset = 0;
    char *buffer = data;

    if (!target)
        target = callbacks[event].target;

    if (!target)
        return 0;

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

static unsigned int read(unsigned int source, unsigned int event, struct message_header *header, void *data)
{

    while (channel_pollfrom(source, header, data) != EVENT_CLOSE)
    {

        if (header->event == event)
            return message_datasize(header);
        else
            channel_dispatch(header, data);

    }

    return 0;

}

static unsigned int redirect(unsigned int target, unsigned int event, unsigned int mode, unsigned int id)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = mode;
    redirect.id = id;

    return send(target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_dispatch(struct message_header *header, void *data)
{

    if (header->event < CHANNEL_CALLBACKS)
    {

        if (callbacks[header->event].callback)
            callbacks[header->event].callback(header->source, data, message_datasize(header));

    }

}

unsigned int channel_send(unsigned int event)
{

    return send(0, event, 0, 0);

}

unsigned int channel_sendto(unsigned int target, unsigned int event)
{

    return send(target, event, 0, 0);

}

unsigned int channel_sendbuffer(unsigned int event, unsigned int count, void *data)
{

    return send(0, event, count, data);

}

unsigned int channel_sendbufferto(unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return send(target, event, count, data);

}

unsigned int channel_sendstringz(unsigned int event, char *string)
{

    return send(0, event, cstring_lengthzero(string), string);

}

unsigned int channel_sendstringzto(unsigned int target, unsigned int event, char *string)
{

    return send(target, event, cstring_lengthzero(string), string);

}

unsigned int channel_sendfmt0(unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return send(0, event, cstring_writefmt0(buffer, MESSAGE_SIZE, fmt, 0), buffer);

}

unsigned int channel_sendfmt0to(unsigned int target, unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt0(buffer, MESSAGE_SIZE, fmt, 0), buffer);

}

unsigned int channel_sendfmt1(unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return send(0, event, cstring_writefmt1(buffer, MESSAGE_SIZE, fmt, 0, arg1), buffer);

}

unsigned int channel_sendfmt1to(unsigned int target, unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt1(buffer, MESSAGE_SIZE, fmt, 0, arg1), buffer);

}

unsigned int channel_sendfmt2(unsigned int event, char *fmt, void *arg1, void *arg2)
{

    char buffer[MESSAGE_SIZE];

    return send(0, event, cstring_writefmt2(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2), buffer);

}

unsigned int channel_sendfmt3(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3)
{

    char buffer[MESSAGE_SIZE];

    return send(0, event, cstring_writefmt3(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3), buffer);

}

unsigned int channel_sendfmt4(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4)
{

    char buffer[MESSAGE_SIZE];

    return send(0, event, cstring_writefmt4(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4), buffer);

}

unsigned int channel_sendfmt6(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{

    char buffer[MESSAGE_SIZE];

    return send(0, event, cstring_writefmt6(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6), buffer);

}

unsigned int channel_sendfmt8(unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8)
{

    char buffer[MESSAGE_SIZE];

    return send(0, event, cstring_writefmt8(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8), buffer);

}

unsigned int channel_redirecttarget(unsigned int target, unsigned int event, unsigned int id)
{

    return redirect(target, event, EVENT_REDIRECT_TARGET, id);

}

unsigned int channel_redirectback(unsigned int target, unsigned int event)
{

    return redirect(target, event, EVENT_REDIRECT_SOURCE, 0);

}

unsigned int channel_pick(struct message_header *header, void *data)
{

    while (active)
    {

        if (call_pick(header, data))
            return header->event;

    }

    return 0;

}

unsigned int channel_process(void)
{

    struct message_header header;
    struct message_data data;

    if (channel_pick(&header, &data))
    {

        channel_dispatch(&header, &data);

        return header.event;

    }

    return 0;

}

unsigned int channel_pollfrom(unsigned int source, struct message_header *header, void *data)
{

    while (channel_pick(header, data))
    {

        if (header->source == source)
            return header->event;

        channel_dispatch(header, data);

    }

    return 0;

}

unsigned int channel_pollevent(unsigned int event, struct message_header *header, void *data)
{

    while (channel_pick(header, data))
    {

        if (header->event == event)
            return header->event;

        channel_dispatch(header, data);

    }

    return 0;

}

unsigned int channel_polleventfrom(unsigned int source, unsigned int event, struct message_header *header, void *data)
{

    while (channel_pick(header, data))
    {

        if (header->source == source && header->event == event)
            return header->event;

        channel_dispatch(header, data);

    }

    return 0;

}

unsigned int channel_kpoll(struct message_header *header, void *data)
{

    return channel_pollfrom(0, header, data);

}

unsigned int channel_kpollevent(unsigned int event, struct message_header *header, void *data)
{

    return channel_polleventfrom(0, event, header, data);

}

unsigned int channel_read(unsigned int event, struct message_header *header, void *data)
{

    return read(0, event, header, data);

}

unsigned int channel_readfrom(unsigned int source, unsigned int event, struct message_header *header, void *data)
{

    return read(source, event, header, data);

}

unsigned int channel_wait(unsigned int source, unsigned int event)
{

    struct message_header header;
    struct message_data data;

    return channel_polleventfrom(source, event, &header, &data);

}

void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize))
{

    callbacks[event].callback = callback;

}

void channel_route(unsigned int event, unsigned int mode, unsigned int id, unsigned int source)
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

void channel_open(void)
{

    active = 1;

}

void channel_close(void)
{

    channel_send(EVENT_CLOSE);

    active = 0;

}

void channel_error(char *description)
{

    channel_sendbuffer(EVENT_ERROR, cstring_length(description), description);
    channel_close();
    call_despawn();

}

void channel_warning(char *description)
{

    channel_sendbuffer(EVENT_ERROR, cstring_length(description), description);

}

