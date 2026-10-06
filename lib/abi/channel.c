#include <fudge.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_STATE_OPENED            1
#define CHANNEL_STATE_CLOSING           2
#define CHANNEL_STATE_CLOSED            3

static void (*listeners[CHANNEL_EVENTS])(struct message *message);
static unsigned int state = CHANNEL_STATE_OPENED;
static unsigned int pipeowner;
static unsigned int pipeprev;
static unsigned int pipenext;

static unsigned int reroute(unsigned int target, unsigned int event)
{

    if (pipenext && (target == pipeowner || (pipeprev && target == pipeprev)))
    {

        switch (event)
        {

        case EVENT_DATA:
            return pipenext;

        case EVENT_ERROR:
            return pipeowner;

        }

    }

    return target;

}

static unsigned int pick(unsigned int ichannel, struct message *message)
{

    while (ichannel || state != CHANNEL_STATE_CLOSED)
    {

        unsigned int status = call_pick(ichannel, message);

        switch (status)
        {

        case MESSAGE_RETRY:
            continue;

        case MESSAGE_OK:
            return message->event;

        case MESSAGE_FAILED:
            return 0;

        }

    }

    return 0;

}

static unsigned int place(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    for (;;)
    {

        unsigned int status = call_place(ichannel, target, event, count, data);

        switch (status)
        {

        case MESSAGE_RETRY:
            continue;

        case MESSAGE_OK:
            return event;

        case MESSAGE_FAILED:
            return 0;

        }

    }

    return 0;

}

static void dispatchpaths(struct message *message)
{

    char *data = message->data;
    unsigned int offset = 0;

    while (offset < message->length && listeners[EVENT_PATH] && state != CHANNEL_STATE_CLOSED)
    {

        struct message path = *message;
        unsigned int length = buffer_findbyte(data + offset, message->length - offset, '\0');

        path.data = data + offset;
        path.length = (offset + length < message->length) ? length + 1 : length;

        listeners[EVENT_PATH](&path);

        offset += length + 1;

    }

}

static void dispatch(struct message *message)
{

    if (message->event == EVENT_PATH)
        dispatchpaths(message);
    else if (message->event < CHANNEL_EVENTS && listeners[message->event])
        listeners[message->event](message);

    switch (message->event)
    {

    case EVENT_TERM:
        if (state == CHANNEL_STATE_OPENED)
            state = CHANNEL_STATE_CLOSING;

        break;

    case EVENT_INTERRUPT:
        state = CHANNEL_STATE_CLOSED;

        break;

    }

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return place(ichannel, reroute(target, event), event, count, data);

}

unsigned int channel_send_fmt0(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmt0(buffer, MESSAGE_SIZE, 0, fmt), buffer);

}

unsigned int channel_send_fmt1(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmt1(buffer, MESSAGE_SIZE, 0, fmt, arg1), buffer);

}

unsigned int channel_send_fmt2(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmt2(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2), buffer);

}

unsigned int channel_send_fmt3(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmt3(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3), buffer);

}

unsigned int channel_send_fmt4(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmt4(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4), buffer);

}

unsigned int channel_send_fmt6(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmt6(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4, arg5, arg6), buffer);

}

unsigned int channel_send_fmt8(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmt8(buffer, MESSAGE_SIZE, 0, fmt, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8), buffer);

}

unsigned int channel_pick(unsigned int ichannel, struct message *message)
{

    return pick(ichannel, message);

}

unsigned int channel_process(unsigned int ichannel)
{

    struct message message;

    if (pick(ichannel, &message))
    {

        dispatch(&message);

        return message.event;

    }

    return 0;

}

void channel_hold(unsigned int ichannel)
{

    while (state != CHANNEL_STATE_CLOSED && channel_process(ichannel));

}

void channel_loop(unsigned int ichannel)
{

    while (state == CHANNEL_STATE_OPENED && channel_process(ichannel));

}

unsigned int channel_poll(unsigned int ichannel, unsigned int source, unsigned int event, struct message *message)
{

    while (pick(ichannel, message))
    {

        dispatch(message);

        if (message->source == source)
        {

            if (message->event == event)
                return event;

            if (message->event == EVENT_EXIT)
                return 0;

        }

    }

    return 0;

}

unsigned int channel_wait(unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct message message;

    return (channel_poll(ichannel, source, event, &message)) ? buffer_read(data, count, message.data, message.length, 0) : 0;

}

unsigned int channel_lookup(char *name)
{

    unsigned int length = cstring_length(name);
    unsigned int offset = buffer_eachbyte(name, length, ':', 0);
    unsigned int index = 0;

    if (offset > 0)
    {

        length = offset - 1;
        index = name[offset] - '0';

    }

    return call_find(length, name, index);

}

void channel_bind(unsigned int event, void (*callback)(struct message *message))
{

    listeners[event] = callback;

}

void channel_pipe(unsigned int owner, unsigned int prev, unsigned int next)
{

    pipeowner = owner;
    pipeprev = prev;
    pipenext = next;

}

void channel_close(void)
{

    state = CHANNEL_STATE_CLOSED;

}

void channel_exit(unsigned int ichannel)
{

    if (pipenext && pipenext != pipeowner)
        place(ichannel, pipenext, EVENT_TERM, 0, 0);

}

