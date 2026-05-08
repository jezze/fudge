#include <fudge.h>
#include <hash.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_STATE_CLOSED            0
#define CHANNEL_STATE_OPENED            1
#define CHANNEL_STATE_CLOSING           2
#define CHANNEL_STATE_TERMINATED        3

static void (*listeners[CHANNEL_EVENTS])(unsigned int source, void *data, unsigned int size);
static unsigned int routes[CHANNEL_EVENTS];
static unsigned int state = CHANNEL_STATE_CLOSED;
static unsigned int pending;
static unsigned int parent;

static unsigned int reroute(unsigned int target, unsigned int event)
{

    return (event < CHANNEL_EVENTS && routes[event]) ? routes[event] : target;

}

static unsigned int send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return channel_place(ichannel, reroute(target, event), event, count, data);

}

static void dispatch(unsigned int source, unsigned int event, void *data, unsigned int size)
{

    if (event < CHANNEL_EVENTS && listeners[event])
    {

        pending++;

        listeners[event](source, data, size);

        pending--;

    }

}

unsigned int channel_pick(unsigned int ichannel, struct message *message, unsigned int count, void *data)
{

    while (state != CHANNEL_STATE_CLOSED && state != CHANNEL_STATE_TERMINATED)
    {

        unsigned int status = call_pick(ichannel, message, count, data);

        switch (status)
        {

        case MESSAGE_OK:
            return message->event;

        case MESSAGE_RETRY:
            continue;

        case MESSAGE_TOOBIG:
            continue;

        case MESSAGE_FAILED:
            return 0;

        case MESSAGE_UNIMPLEMENTED:
            return 0;

        }

    }

    return 0;

}

unsigned int channel_place(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    while (state != CHANNEL_STATE_CLOSED)
    {

        unsigned int status = call_place(ichannel, target, event, count, data);

        switch (status)
        {

        case MESSAGE_OK:
            return event;

        case MESSAGE_RETRY:
            continue;

        case MESSAGE_TOOBIG:
            return 0;

        case MESSAGE_FAILED:
            return 0;

        case MESSAGE_UNIMPLEMENTED:
            return 0;

        }

    }

    return 0;

}

void channel_dispatch(unsigned int ichannel, struct message *message, void *data)
{

    dispatch(message->source, message->event, data, message->length);

    switch (message->event)
    {

    case EVENT_TERM:
        parent = message->source;
        state = CHANNEL_STATE_CLOSING;

        break;

    }

    if (state == CHANNEL_STATE_CLOSING)
    {

        if (!pending && parent)
            dispatch(parent, EVENT_EXIT, 0, 0);

        if (!pending)
            state = CHANNEL_STATE_TERMINATED;

    }

    if (state == CHANNEL_STATE_TERMINATED)
    {

        if (parent)
            channel_send(ichannel, parent, EVENT_DONE);

    }

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event)
{

    return send(ichannel, target, event, 0, 0);

}

unsigned int channel_send_buffer(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return send(ichannel, target, event, count, data);

}

unsigned int channel_send_fmt0(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return send(ichannel, target, event, cstring_write_fmt0(buffer, MESSAGE_SIZE, 0, fmt), buffer);

}

unsigned int channel_send_fmt1(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return send(ichannel, target, event, cstring_write_fmt1(buffer, MESSAGE_SIZE, 0, fmt, arg1), buffer);

}

unsigned int channel_send_fmt2(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2)
{

    char buffer[MESSAGE_SIZE];

    return send(ichannel, target, event, cstring_write_fmt2(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2), buffer);

}

unsigned int channel_send_fmt3(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3)
{

    char buffer[MESSAGE_SIZE];

    return send(ichannel, target, event, cstring_write_fmt3(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3), buffer);

}

unsigned int channel_send_fmt4(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4)
{

    char buffer[MESSAGE_SIZE];

    return send(ichannel, target, event, cstring_write_fmt4(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4), buffer);

}

unsigned int channel_send_fmt6(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{

    char buffer[MESSAGE_SIZE];

    return send(ichannel, target, event, cstring_write_fmt6(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4, arg5, arg6), buffer);

}

unsigned int channel_send_fmt8(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8)
{

    char buffer[MESSAGE_SIZE];

    return send(ichannel, target, event, cstring_write_fmt8(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8), buffer);

}

unsigned int channel_process(unsigned int ichannel)
{

    struct message message;
    char data[MESSAGE_SIZE];

    if (channel_pick(ichannel, &message, MESSAGE_SIZE, data))
    {

        channel_dispatch(ichannel, &message, data);

        return message.event;

    }

    return 0;

}

unsigned int channel_poll(unsigned int ichannel, unsigned int source, unsigned int event, struct message *message, unsigned int count, void *data)
{

    while (channel_pick(ichannel, message, count, data))
    {

        channel_dispatch(ichannel, message, data);

        if (message->source == source && message->event == event)
            return message->event;

    }

    return 0;

}

unsigned int channel_pollany(unsigned int ichannel, unsigned int source, struct message *message, unsigned int count, void *data)
{

    while (channel_pick(ichannel, message, count, data))
    {

        channel_dispatch(ichannel, message, data);

        if (message->source == source)
            return message->event;

    }

    return 0;

}

unsigned int channel_wait(unsigned int ichannel, unsigned int source, unsigned int event)
{

    struct message message;
    char data[MESSAGE_SIZE];

    while (channel_pick(ichannel, &message, MESSAGE_SIZE, data))
    {

        channel_dispatch(ichannel, &message, data);

        if (message.source == source && message.event == event)
            return message.event;

    }

    return 0;

}

unsigned int channel_wait_buffer(unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct message message;
    char data2[MESSAGE_SIZE];

    while (channel_pick(ichannel, &message, MESSAGE_SIZE, data2))
    {

        channel_dispatch(ichannel, &message, data2);

        if (message.source == source && message.event == event)
        {

            buffer_write(data, count, data2, message.length, 0);

            return message.event;

        }

    }

    return 0;

}

unsigned int channel_lookup(char *name)
{

    unsigned int length = cstring_length(name);
    unsigned int offset = buffer_eachbyte(name, length, ':', 0);

    if (offset > 0)
    {

        unsigned int namehash = djb_hash(offset - 1, name);
        unsigned int index = name[offset] - '0';

        return call_find(namehash, index);

    }

    else
    {

        unsigned int namehash = djb_hash(length, name);

        return call_find(namehash, 0);

    }

    return 0;

}

void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize))
{

    listeners[event] = callback;

}

void channel_route(unsigned int event, unsigned int target)
{

    routes[event] = target;

}

void channel_open(void)
{

    state = CHANNEL_STATE_OPENED;

}

void channel_close(void)
{

    state = CHANNEL_STATE_CLOSED;

}

