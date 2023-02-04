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

    struct message message;
    unsigned int offset = 0;
    char *buffer = data;

    if (!target)
        target = callbacks[event].target;

    if (!target)
        return 0;

    while (count > MESSAGE_SIZE)
    {

        message_init(&message, event, MESSAGE_SIZE);

        while (!call_place(target, &message, buffer + offset));

        count -= MESSAGE_SIZE;
        offset += MESSAGE_SIZE;

    }

    message_init(&message, event, count);

    while (!call_place(target, &message, buffer + offset));

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

void channel_dispatch(struct message *message, void *data)
{

    if (message->event < CHANNEL_CALLBACKS)
    {

        if (callbacks[message->event].callback)
            callbacks[message->event].callback(message->source, data, message_datasize(message));

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

unsigned int channel_sendfmt0(unsigned int target, unsigned int event, char *fmt)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt0(buffer, MESSAGE_SIZE, fmt, 0), buffer);

}

unsigned int channel_sendfmt1(unsigned int target, unsigned int event, char *fmt, void *arg1)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt1(buffer, MESSAGE_SIZE, fmt, 0, arg1), buffer);

}

unsigned int channel_sendfmt2(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt2(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2), buffer);

}

unsigned int channel_sendfmt3(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt3(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3), buffer);

}

unsigned int channel_sendfmt4(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt4(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4), buffer);

}

unsigned int channel_sendfmt6(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt6(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6), buffer);

}

unsigned int channel_sendfmt8(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8)
{

    char buffer[MESSAGE_SIZE];

    return send(target, event, cstring_writefmt8(buffer, MESSAGE_SIZE, fmt, 0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8), buffer);

}

unsigned int channel_listen(unsigned int target, unsigned int event)
{

    return redirect(target, event, EVENT_REDIRECT_SOURCE, CHANNEL_DEFAULT);

}

unsigned int channel_forward(unsigned int target, unsigned int event, unsigned int id)
{

    return redirect(target, event, EVENT_REDIRECT_TARGET, id);

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

unsigned int channel_poll(unsigned int event, struct message *message, void *data)
{

    while (channel_pick(message, data))
    {

        if (message->event == event)
            return message->event;

        channel_dispatch(message, data);

    }

    return 0;

}

unsigned int channel_pollfrom(unsigned int source, unsigned int event, struct message *message, void *data)
{

    while (channel_pick(message, data))
    {

        if (message->event == event && message->source == source)
            return message->event;

        channel_dispatch(message, data);

    }

    return 0;

}

unsigned int channel_read(unsigned int event, void *data)
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

unsigned int channel_readfrom(unsigned int source, unsigned int event, void *data)
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

unsigned int channel_wait(unsigned int source, unsigned int event)
{

    struct message message;
    char data[MESSAGE_SIZE];

    return channel_pollfrom(source, event, &message, data);

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
        callbacks[event].target = CHANNEL_DEFAULT;

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

void channel_error(char *description)
{

    send(CHANNEL_DEFAULT, EVENT_ERROR, cstring_length(description), description);
    send(CHANNEL_DEFAULT, EVENT_CLOSE, 0, 0);
    call_despawn();

}

void channel_warning(char *description)
{

    send(CHANNEL_DEFAULT, EVENT_ERROR, cstring_length(description), description);

}

