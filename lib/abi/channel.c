#include <fudge.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_LISTENERS               256
#define CHANNEL_STATE_CLOSED            0
#define CHANNEL_STATE_OPENED            1
#define CHANNEL_STATE_WAITING           2
#define CHANNEL_STATE_TERMINATED        3

static struct listener
{

    void (*callback)(unsigned int source, void *data, unsigned int size);
    unsigned int target;
    unsigned int autoclose;

} listeners[CHANNEL_LISTENERS];

static unsigned int state = CHANNEL_STATE_CLOSED;
static unsigned int pending;
static unsigned int parent;

static unsigned int send(unsigned int channel, unsigned int event, unsigned int count, void *data)
{

    struct listener *listener = &listeners[event];

    if (listener->target)
        channel = listener->target;

    if (!channel)
        return 0;

    while (!call_place(channel, event, count, data));

    return count;

}

static unsigned int redirect(unsigned int channel, unsigned int event, unsigned int mode, unsigned int target)
{

    struct event_redirect redirect;

    redirect.event = event;
    redirect.mode = mode;
    redirect.target = target;

    return send(channel, EVENT_REDIRECT, sizeof (struct event_redirect), &redirect);

}

void channel_dispatch(struct message *message, void *data)
{

    if (message->event < CHANNEL_LISTENERS)
    {

        struct listener *listener = &listeners[message->event];

        if (listener->callback)
        {

            pending++;

            listener->callback(message->source, data, message_datasize(message));

            pending--;

        }

        switch (listener->autoclose)
        {

        case 1:
            parent = message->source;
            state = CHANNEL_STATE_WAITING;

            break;

        case 2:
            parent = message->source;
            state = CHANNEL_STATE_TERMINATED;

            break;

        }

        if (state == CHANNEL_STATE_WAITING && !pending)
            state = CHANNEL_STATE_TERMINATED;

        if (state == CHANNEL_STATE_TERMINATED)
        {

            if (parent)
            {

                struct listener *exitlistener = &listeners[EVENT_EXIT];

                if (exitlistener->callback)
                    exitlistener->callback(parent, 0, 0);

                send(parent, EVENT_TERMRESPONSE, 0, 0);

            }

            channel_close();

        }

    }

}

unsigned int channel_send(unsigned int channel, unsigned int event)
{

    return send(channel, event, 0, 0);

}

unsigned int channel_send_buffer(unsigned int channel, unsigned int event, unsigned int count, void *data)
{

    return send(channel, event, count, data);

}

unsigned int channel_send_fmt0(unsigned int channel, unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return send(channel, event, cstring_write_fmt0(buffer, MESSAGE_SIZE, fmt, 0), buffer);

}

unsigned int channel_send_fmt1(unsigned int channel, unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return send(channel, event, cstring_write_fmt1(buffer, MESSAGE_SIZE, fmt, 0, arg1), buffer);

}

unsigned int channel_send_fmt2(unsigned int channel, unsigned int event, char *fmt, void *arg1, void *arg2)
{

    char buffer[MESSAGE_SIZE];

    return send(channel, event, cstring_write_fmt2(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2), buffer);

}

unsigned int channel_send_fmt3(unsigned int channel, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3)
{

    char buffer[MESSAGE_SIZE];

    return send(channel, event, cstring_write_fmt3(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3), buffer);

}

unsigned int channel_send_fmt4(unsigned int channel, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4)
{

    char buffer[MESSAGE_SIZE];

    return send(channel, event, cstring_write_fmt4(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4), buffer);

}

unsigned int channel_send_fmt6(unsigned int channel, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{

    char buffer[MESSAGE_SIZE];

    return send(channel, event, cstring_write_fmt6(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6), buffer);

}

unsigned int channel_send_fmt8(unsigned int channel, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8)
{

    char buffer[MESSAGE_SIZE];

    return send(channel, event, cstring_write_fmt8(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8), buffer);

}

unsigned int channel_listen(unsigned int channel, unsigned int event)
{

    return redirect(channel, event, EVENT_REDIRECT_SOURCE, 0);

}

unsigned int channel_forward(unsigned int channel, unsigned int event, unsigned int target)
{

    return redirect(channel, event, EVENT_REDIRECT_TARGET, target);

}

unsigned int channel_pick(struct message *message, unsigned int count, void *data)
{

    while (state != CHANNEL_STATE_CLOSED)
    {

        if (call_pick(message, count, data))
            return message->event;

    }

    return 0;

}

unsigned int channel_process(void)
{

    struct message message;
    char data[MESSAGE_SIZE];

    if (channel_pick(&message, MESSAGE_SIZE, data))
    {

        channel_dispatch(&message, data);

        return message.event;

    }

    return 0;

}

unsigned int channel_poll_any(unsigned int event, struct message *message, unsigned int count, void *data)
{

    while (channel_pick(message, count, data))
    {

        if (message->event == event)
            return message->event;

        channel_dispatch(message, data);

    }

    return 0;

}

unsigned int channel_read_any(unsigned int event, unsigned int count, void *data)
{

    struct message message;

    while (channel_pick(&message, count, data) != EVENT_TERMRESPONSE)
    {

        if (message.event == event)
            return message_datasize(&message);

        channel_dispatch(&message, data);

    }

    return 0;

}

unsigned int channel_wait_any(unsigned int event)
{

    struct message message;
    char data[MESSAGE_SIZE];

    return channel_poll_any(event, &message, MESSAGE_SIZE, data);

}

void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize))
{

    struct listener *listener = &listeners[event];

    listener->callback = callback;

}

void channel_autoclose(unsigned int event, unsigned int autoclose)
{

    struct listener *listener = &listeners[event];

    listener->autoclose = autoclose;

}

void channel_route(unsigned int event, unsigned int mode, unsigned int target, unsigned int source)
{

    struct listener *listener = &listeners[event];

    switch (mode)
    {

    case EVENT_REDIRECT_TARGET:
        listener->target = target;

        break;

    case EVENT_REDIRECT_SOURCE:
        listener->target = source;

        break;

    default:
        listener->target = 0;

        break;

    }

}

void channel_open(void)
{

    state = CHANNEL_STATE_OPENED;

}

void channel_close(void)
{

    state = CHANNEL_STATE_CLOSED;

}

