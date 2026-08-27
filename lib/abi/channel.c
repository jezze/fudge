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
static unsigned int state = CHANNEL_STATE_OPENED;
static unsigned int pending;

static unsigned int reroute(unsigned int target, unsigned int event)
{

    return (event < CHANNEL_EVENTS && routes[event]) ? routes[event] : target;

}

unsigned int channel_pick(unsigned int ichannel, struct message *message)
{

    while (state != CHANNEL_STATE_CLOSED && state != CHANNEL_STATE_TERMINATED)
    {

        unsigned int status = call_pick(ichannel, message);

        switch (status)
        {

        case MESSAGE_OK:
            return message->event;

        case MESSAGE_RETRY:
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

        case MESSAGE_FAILED:
            return 0;

        case MESSAGE_UNIMPLEMENTED:
            return 0;

        }

    }

    return 0;

}

void channel_dispatch(unsigned int ichannel, struct message *message)
{

    if (message->event < CHANNEL_EVENTS && listeners[message->event])
    {

        char data[MESSAGE_SIZE];
        unsigned int count = buffer_read(data, MESSAGE_SIZE, message_data(message, ichannel), message->length, 0);

        pending++;

        listeners[message->event](message->source, data, count);

        pending--;

    }

    switch (message->event)
    {

    case EVENT_TERM:
        channel_route(EVENT_DONE, message->source);

        state = CHANNEL_STATE_CLOSING;

        break;

    case EVENT_KILL:
        channel_route(EVENT_DONE, message->source);

        state = CHANNEL_STATE_TERMINATED;

        break;

    }

    if (state == CHANNEL_STATE_CLOSING)
    {

        if (!pending)
            state = CHANNEL_STATE_TERMINATED;

    }

    if (state == CHANNEL_STATE_TERMINATED)
    {

        channel_place(ichannel, reroute(message->source, EVENT_DONE), EVENT_DONE, 0, 0);

    }

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return channel_place(ichannel, reroute(target, event), event, count, data);

}

unsigned int channel_send_fmt0(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return channel_place(ichannel, reroute(target, event), event, cstring_write_fmt0(buffer, MESSAGE_SIZE, 0, fmt), buffer);

}

unsigned int channel_send_fmt1(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return channel_place(ichannel, reroute(target, event), event, cstring_write_fmt1(buffer, MESSAGE_SIZE, 0, fmt, arg1), buffer);

}

unsigned int channel_send_fmt2(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2)
{

    char buffer[MESSAGE_SIZE];

    return channel_place(ichannel, reroute(target, event), event, cstring_write_fmt2(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2), buffer);

}

unsigned int channel_send_fmt3(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3)
{

    char buffer[MESSAGE_SIZE];

    return channel_place(ichannel, reroute(target, event), event, cstring_write_fmt3(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3), buffer);

}

unsigned int channel_send_fmt4(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4)
{

    char buffer[MESSAGE_SIZE];

    return channel_place(ichannel, reroute(target, event), event, cstring_write_fmt4(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4), buffer);

}

unsigned int channel_send_fmt6(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{

    char buffer[MESSAGE_SIZE];

    return channel_place(ichannel, reroute(target, event), event, cstring_write_fmt6(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4, arg5, arg6), buffer);

}

unsigned int channel_send_fmt8(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8)
{

    char buffer[MESSAGE_SIZE];

    return channel_place(ichannel, reroute(target, event), event, cstring_write_fmt8(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8), buffer);

}

unsigned int channel_process(unsigned int ichannel)
{

    struct message message;

    if (channel_pick(ichannel, &message))
    {

        channel_dispatch(ichannel, &message);

        return message.event;

    }

    return 0;

}

unsigned int channel_poll(unsigned int ichannel, unsigned int source, unsigned int event, struct message *message)
{

    while (channel_pick(ichannel, message))
    {

        channel_dispatch(ichannel, message);

        if (message->source == source && (message->event == event || event == EVENT_ALL))
            return message->event;

    }

    return 0;

}

unsigned int channel_wait(unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct message message;

    channel_poll(ichannel, source, event, &message);

    return buffer_read(data, count, message_data(&message, ichannel), message.length, 0);

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

void channel_close(void)
{

    state = CHANNEL_STATE_CLOSED;

}

