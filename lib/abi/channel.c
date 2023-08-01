#include <fudge.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_LISTENERS 256

static struct
{

    void (*callback)(unsigned int source, void *data, unsigned int size);
    unsigned int target;
    unsigned int autoclose;

} listeners[CHANNEL_LISTENERS];

static unsigned int active;

static unsigned int send(unsigned int target, unsigned int event, unsigned int count, void *data)
{

    if (count > MESSAGE_SIZE)
        return 0;

    if (target == CHANNEL_DEFAULT)
        target = listeners[event].target;

    if (!target)
        return 0;

    while (!call_place(target, event, count, data));

    return count;

}

static unsigned int redirect(unsigned int target, unsigned int event, unsigned int mode, unsigned int channel)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = mode;
    redirect.target = channel;

    return send(target, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_dispatch(struct message *message, void *data)
{

    if (message->event < CHANNEL_LISTENERS)
    {

        if (listeners[message->event].callback)
            listeners[message->event].callback(message->source, data, message_datasize(message));

        if (listeners[message->event].autoclose)
            channel_close();

    }

}

unsigned int channel_send(unsigned int target, unsigned int event)
{

    return send(target, event, 0, 0);

}

unsigned int channel_send_buffer(unsigned int target, unsigned int event, unsigned int count, void *data)
{

    return send(target, event, count, data);

}

unsigned int channel_send_fmt0(unsigned int target, unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_write_fmt0(buffer, MESSAGE_SIZE, fmt, 0), buffer);

}

unsigned int channel_send_fmt1(unsigned int target, unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_write_fmt1(buffer, MESSAGE_SIZE, fmt, 0, arg1), buffer);

}

unsigned int channel_send_fmt2(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_write_fmt2(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2), buffer);

}

unsigned int channel_send_fmt3(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_write_fmt3(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3), buffer);

}

unsigned int channel_send_fmt4(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_write_fmt4(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4), buffer);

}

unsigned int channel_send_fmt6(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_write_fmt6(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6), buffer);

}

unsigned int channel_send_fmt8(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_write_fmt8(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8), buffer);

}

unsigned int channel_listen(unsigned int target, unsigned int event)
{

    return redirect(target, event, EVENT_REDIRECT_SOURCE, CHANNEL_DEFAULT);

}

unsigned int channel_forward(unsigned int target, unsigned int event, unsigned int channel)
{

    return redirect(target, event, EVENT_REDIRECT_TARGET, channel);

}

unsigned int channel_pick(struct message *message, void *data)
{

    while (active)
    {

        if (call_pick(message, data))
            return message->event;

    }

    return 0;

}

unsigned int channel_process(void)
{

    struct message message;
    char data[MESSAGE_SIZE];

    if (channel_pick(&message, data))
    {

        channel_dispatch(&message, data);

        return message.event;

    }

    return 0;

}

unsigned int channel_poll_any(unsigned int event, struct message *message, void *data)
{

    while (channel_pick(message, data))
    {

        if (message->event == event)
            return message->event;

        channel_dispatch(message, data);

    }

    return 0;

}

unsigned int channel_poll_from(unsigned int source, unsigned int event, struct message *message, void *data)
{

    while (channel_pick(message, data))
    {

        if (message->event == event && message->source == source)
            return message->event;

        channel_dispatch(message, data);

    }

    return 0;

}

unsigned int channel_read_any(unsigned int event, void *data)
{

    struct message message;

    while (channel_pick(&message, data) != EVENT_CLOSE)
    {

        if (message.event == event)
            return message_datasize(&message);

        channel_dispatch(&message, data);

    }

    return 0;

}

unsigned int channel_read_from(unsigned int source, unsigned int event, void *data)
{

    struct message message;

    while (channel_pick(&message, data) != EVENT_CLOSE)
    {

        if (message.event == event && message.source == source)
            return message_datasize(&message);

        channel_dispatch(&message, data);

    }

    return 0;

}

unsigned int channel_wait_any(unsigned int event)
{

    struct message message;
    char data[MESSAGE_SIZE];

    return channel_poll_any(event, &message, data);

}

unsigned int channel_wait_from(unsigned int source, unsigned int event)
{

    struct message message;
    char data[MESSAGE_SIZE];

    return channel_poll_from(source, event, &message, data);

}

void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize))
{

    listeners[event].callback = callback;

}

void channel_autoclose(unsigned int event, unsigned int autoclose)
{

    listeners[event].autoclose = autoclose;

}

void channel_route(unsigned int event, unsigned int mode, unsigned int target, unsigned int source)
{

    switch (mode)
    {

    case EVENT_REDIRECT_TARGET:
        listeners[event].target = target;

        break;

    case EVENT_REDIRECT_SOURCE:
        listeners[event].target = source;

        break;

    default:
        listeners[event].target = CHANNEL_DEFAULT;

        break;

    }

}

void channel_open(void)
{

    active = 1;

}

void channel_close(void)
{

    send(CHANNEL_DEFAULT, EVENT_CLOSE, 0, 0);

    active = 0;

}

