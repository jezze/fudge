#include <fudge.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_MAILBOXES               8
#define CHANNEL_COPIES                  16
#define CHANNEL_QUEUE                   32
#define CHANNEL_STATE_OPENED            0
#define CHANNEL_STATE_CLOSING           1
#define CHANNEL_STATE_CLOSED            2

struct inbox
{

    struct message messages[CHANNEL_QUEUE];
    unsigned int head;
    unsigned int count;
    unsigned int release;
    unsigned int handed;
    unsigned int opened;
    unsigned int state;

};

static void (*listeners[CHANNEL_EVENTS])(struct message *message);

static struct inbox inboxes[CHANNEL_MAILBOXES];
static char copies[CHANNEL_COPIES][MESSAGE_SIZE];
static unsigned int copiesused;
static unsigned int owner;
static struct event_pipe pipe;

static struct event_route *findroute(unsigned int event)
{

    unsigned int i;

    for (i = 0; i < pipe.nroutes; i++)
    {

        if (pipe.routes[i].event == event)
            return &pipe.routes[i];

    }

    return 0;

}

static unsigned int resolve(unsigned int target)
{

    return (target) ? target : owner;

}

static void setpipe(struct message *message)
{

    owner = message->source;

    if (message->length == sizeof (struct event_pipe))
    {

        buffer_copy(&pipe, message->data, sizeof (struct event_pipe));

        if (pipe.nroutes > PIPE_ROUTES)
            pipe.nroutes = PIPE_ROUTES;

        if (pipe.nterms > PIPE_ROUTES)
            pipe.nterms = PIPE_ROUTES;

    }

    else
    {

        buffer_clear(&pipe, sizeof (struct event_pipe));

        pipe.nterms = 1;
        pipe.terms[0].event = EVENT_TERM;
        pipe.terms[0].to = EVENT_TERM;

    }

}

static unsigned int frominput(unsigned int ichannel, unsigned int source)
{

    if (ichannel)
        return 1;

    if (pipe.prev)
        return source == pipe.prev;

    if (owner)
        return source == owner;

    return 1;

}

static void release(struct inbox *inbox, unsigned int slots)
{

    inbox->release |= slots & ((1 << MESSAGE_SLOTS) - 1);
    copiesused &= ~(slots >> MESSAGE_SLOTS);

}

static unsigned int copy(struct inbox *inbox, struct message *message)
{

    unsigned int i;

    for (i = 0; i < CHANNEL_COPIES; i++)
    {

        if (!(copiesused & (1 << i)))
        {

            buffer_copy(copies[i], message->data, message->length);
            release(inbox, 1 << message->slot);

            copiesused |= 1 << i;
            message->data = copies[i];
            message->slot = MESSAGE_SLOTS + i;

            return 1;

        }

    }

    return 0;

}

static unsigned int fill(unsigned int ichannel, struct inbox *inbox, unsigned int flags)
{

    struct message messages[MESSAGE_SLOTS];
    unsigned int status = call_pick(ichannel, messages, MESSAGE_SLOTS, inbox->release, flags);
    unsigned int i;

    inbox->release = 0;
    inbox->opened = 1;

    if (status == MESSAGE_OK)
    {

        for (i = 0; i < MESSAGE_SLOTS && messages[i].event; i++)
            buffer_copy(&inbox->messages[(inbox->head + inbox->count++) % CHANNEL_QUEUE], &messages[i], sizeof (struct message));

    }

    return status;

}

static unsigned int pullinbox(unsigned int ichannel, struct inbox *inbox)
{

    unsigned int freed = inbox->release;
    unsigned int i;

    fill(ichannel, inbox, 0);

    for (i = 0; i < inbox->count; i++)
    {

        struct message *message = &inbox->messages[(inbox->head + i) % CHANNEL_QUEUE];

        if (message->slot < MESSAGE_SLOTS && !copy(inbox, message))
            break;

    }

    if (inbox->release)
    {

        freed |= inbox->release;

        call_pick(ichannel, 0, 0, inbox->release, 0);

        inbox->release = 0;

    }

    return freed;

}

static unsigned int pull(void)
{

    unsigned int freed = 0;
    unsigned int i;

    for (i = 0; i < CHANNEL_MAILBOXES; i++)
    {

        if (inboxes[i].opened)
            freed |= pullinbox(i, &inboxes[i]);

    }

    return freed;

}

static unsigned int take(unsigned int ichannel, struct message *message, unsigned int flags)
{

    struct inbox *inbox = &inboxes[ichannel];

    release(inbox, inbox->handed);

    inbox->handed = 0;

    if (!inbox->count)
    {

        unsigned int status = fill(ichannel, inbox, flags);

        if (status != MESSAGE_OK)
            return status;

    }

    buffer_copy(message, &inbox->messages[inbox->head], sizeof (struct message));

    inbox->head = (inbox->head + 1) % CHANNEL_QUEUE;
    inbox->count--;

    return MESSAGE_OK;

}

static void finish(unsigned int ichannel, struct message *message)
{

    release(&inboxes[ichannel], 1 << message->slot);

}

static void hand(unsigned int ichannel, struct message *message)
{

    inboxes[ichannel].handed |= 1 << message->slot;

}

static unsigned int canpick(unsigned int ichannel)
{

    return ichannel < CHANNEL_MAILBOXES && inboxes[ichannel].state != CHANNEL_STATE_CLOSED;

}

static unsigned int pick(unsigned int ichannel, struct message *message)
{

    while (canpick(ichannel))
    {

        unsigned int status = take(ichannel, message, MESSAGE_FLAG_BLOCK);

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

        case MESSAGE_FULL:
            if (!pull())
                call_pick(ichannel, 0, 0, 0, MESSAGE_FLAG_BLOCK);

            continue;

        case MESSAGE_OK:
            return event;

        case MESSAGE_FAILED:
            return 0;

        }

    }

    return 0;

}

static void dispatch(unsigned int ichannel, struct message *message)
{

    struct inbox *inbox = &inboxes[ichannel];

    if (message->event == EVENT_MAIN && !ichannel)
        setpipe(message);

    if (message->event < CHANNEL_EVENTS && listeners[message->event])
        listeners[message->event](message);

    switch (message->event)
    {

    case EVENT_TERM:
        if (inbox->state == CHANNEL_STATE_OPENED && frominput(ichannel, message->source))
            inbox->state = CHANNEL_STATE_CLOSING;

        break;

    case EVENT_INTERRUPT:
        inbox->state = CHANNEL_STATE_CLOSED;

        break;

    }

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    if (!ichannel && (target == owner || (pipe.prev && target == pipe.prev)))
    {

        struct event_route *route = findroute(event);

        if (route)
            return place(ichannel, resolve(route->target), route->to, count, data);

    }

    return place(ichannel, target, event, count, data);

}

unsigned int channel_send_fmtv(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void **args)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmtv(buffer, MESSAGE_SIZE, 0, fmt, args), buffer);

}

unsigned int channel_pick(unsigned int ichannel, struct message *message)
{

    unsigned int event = pick(ichannel, message);

    if (event)
        hand(ichannel, message);

    return event;

}

unsigned int channel_process(unsigned int ichannel)
{

    struct message message;

    if (pick(ichannel, &message))
    {

        dispatch(ichannel, &message);
        finish(ichannel, &message);

        return message.event;

    }

    return 0;

}

unsigned int channel_drain(unsigned int ichannel)
{

    struct message message;

    while (canpick(ichannel))
    {

        switch (take(ichannel, &message, 0))
        {

        case MESSAGE_OK:
            dispatch(ichannel, &message);
            finish(ichannel, &message);

            break;

        case MESSAGE_RETRY:
            return 1;

        default:
            return 0;

        }

    }

    return 0;

}

void channel_hold(unsigned int ichannel)
{

    while (canpick(ichannel) && channel_process(ichannel));

}

void channel_loop(unsigned int ichannel)
{

    while (ichannel < CHANNEL_MAILBOXES && inboxes[ichannel].state == CHANNEL_STATE_OPENED && channel_process(ichannel));

}

unsigned int channel_poll(unsigned int ichannel, unsigned int source, unsigned int event, struct message *message)
{

    while (pick(ichannel, message))
    {

        dispatch(ichannel, message);

        if (message->source == source && message->event == event)
        {

            hand(ichannel, message);

            return event;

        }

        finish(ichannel, message);

        if (message->source == source && message->event == EVENT_TERM)
            return 0;

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

        index = cstring_read_value(name + offset, length - offset, 10);
        length = offset - 1;

    }

    return call_find(length, name, index);

}

void channel_bind(unsigned int event, void (*callback)(struct message *message))
{

    listeners[event] = callback;

}

void channel_close(unsigned int ichannel)
{

    if (ichannel < CHANNEL_MAILBOXES)
        inboxes[ichannel].state = CHANNEL_STATE_CLOSED;

}

void channel_exit(unsigned int ichannel)
{

    unsigned int i;

    for (i = 0; i < pipe.nterms; i++)
        place(ichannel, resolve(pipe.terms[i].target), pipe.terms[i].to, 0, 0);

}

void channel_abort(unsigned int ichannel)
{

    char *error = "Process aborted\n";

    if (owner)
        channel_send(ichannel, owner, EVENT_ERROR, cstring_length(error), error);

    channel_exit(ichannel);

}
