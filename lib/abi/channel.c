#include <fudge.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_MAILBOXES               8
#define CHANNEL_COPIES                  16
#define CHANNEL_QUEUE                   32
#define CHANNEL_STATE_OPENED            1
#define CHANNEL_STATE_CLOSING           2
#define CHANNEL_STATE_CLOSED            3

/* a slot of MESSAGE_SLOTS or more is a copy */
struct inbox
{

    struct message messages[CHANNEL_QUEUE];
    unsigned int head;
    unsigned int count;
    unsigned int release;
    unsigned int handed;
    unsigned int opened;

};

static void (*listeners[CHANNEL_EVENTS])(struct message *message);
static unsigned int state = CHANNEL_STATE_OPENED;
static unsigned int pipeowner;
static unsigned int pipeprev;
static unsigned int pipenext;
static struct event_route routes[PIPE_ROUTES + 2];
static unsigned int nroutes;
static struct inbox inboxes[CHANNEL_MAILBOXES];
static char copies[CHANNEL_COPIES][MESSAGE_SIZE];
static unsigned int copiesused;

static struct event_route *findroute(unsigned int event)
{

    unsigned int i;

    for (i = 0; i < nroutes; i++)
    {

        if (routes[i].event == event)
            return &routes[i];

    }

    return 0;

}

static unsigned int routetarget(struct event_route *route)
{

    if (route->target)
        return route->target;

    return (route->to == EVENT_ERROR) ? pipeowner : pipenext;

}

static struct event_route *findreroute(unsigned int target, unsigned int event)
{

    if (pipenext && (target == pipeowner || (pipeprev && target == pipeprev)))
        return findroute(event);

    return 0;

}

static void done(struct inbox *inbox, unsigned int slots)
{

    inbox->release |= slots & ((1 << MESSAGE_SLOTS) - 1);
    copiesused &= ~(slots >> MESSAGE_SLOTS);

}

static void append(struct inbox *inbox, struct message *message)
{

    buffer_copy(&inbox->messages[(inbox->head + inbox->count) % CHANNEL_QUEUE], message, sizeof (struct message));

    inbox->count++;

}

static unsigned int copy(struct inbox *inbox, struct message *message)
{

    unsigned int i;

    for (i = 0; i < CHANNEL_COPIES; i++)
    {

        if (!(copiesused & (1 << i)))
        {

            buffer_copy(copies[i], message->data, message->length);

            copiesused |= 1 << i;
            inbox->release |= 1 << message->slot;
            message->data = copies[i];
            message->slot = MESSAGE_SLOTS + i;

            return 1;

        }

    }

    return 0;

}

static unsigned int pullinbox(unsigned int ichannel, struct inbox *inbox)
{

    struct message messages[MESSAGE_SLOTS];
    unsigned int released = inbox->release;
    unsigned int i;

    if (call_pick(ichannel, messages, MESSAGE_SLOTS, inbox->release, 0) == MESSAGE_OK)
    {

        for (i = 0; i < MESSAGE_SLOTS && messages[i].event; i++)
            append(inbox, &messages[i]);

    }

    inbox->release = 0;

    for (i = 0; i < inbox->count; i++)
    {

        struct message *message = &inbox->messages[(inbox->head + i) % CHANNEL_QUEUE];

        if (message->slot < MESSAGE_SLOTS && !copy(inbox, message))
            break;

    }

    if (inbox->release)
        call_pick(ichannel, 0, 0, inbox->release, 0);

    released |= inbox->release;
    inbox->release = 0;

    return released;

}

static unsigned int pull(void)
{

    unsigned int released = 0;
    unsigned int i;

    for (i = 0; i < CHANNEL_MAILBOXES; i++)
    {

        if (inboxes[i].opened)
            released |= pullinbox(i, &inboxes[i]);

    }

    return released;

}

static unsigned int take(unsigned int ichannel, struct message *message, unsigned int flags)
{

    struct inbox *inbox = &inboxes[ichannel];

    inbox->opened = 1;

    done(inbox, inbox->handed);

    inbox->handed = 0;

    if (!inbox->count)
    {

        unsigned int status = call_pick(ichannel, inbox->messages, MESSAGE_SLOTS, inbox->release, flags);

        inbox->release = 0;

        if (status != MESSAGE_OK)
            return status;

        inbox->head = 0;

        while (inbox->count < MESSAGE_SLOTS && inbox->messages[inbox->count].event)
            inbox->count++;

    }

    buffer_copy(message, &inbox->messages[inbox->head], sizeof (struct message));

    inbox->head = (inbox->head + 1) % CHANNEL_QUEUE;
    inbox->count--;

    return MESSAGE_OK;

}

static void finish(unsigned int ichannel, struct message *message)
{

    done(&inboxes[ichannel], 1 << message->slot);

}

static void hand(unsigned int ichannel, struct message *message)
{

    inboxes[ichannel].handed |= 1 << message->slot;

}

static unsigned int canpick(unsigned int ichannel)
{

    return ichannel < CHANNEL_MAILBOXES && (ichannel || state != CHANNEL_STATE_CLOSED);

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

static void dispatchpaths(struct message *message)
{

    char *data = message->data;
    unsigned int offset = 0;

    while (offset < message->length && listeners[EVENT_PATH] && state != CHANNEL_STATE_CLOSED)
    {

        struct message path;
        unsigned int length = buffer_findbyte(data + offset, message->length - offset, '\0');

        path.event = message->event;
        path.source = message->source;
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

    struct event_route *route = (ichannel) ? 0 : findreroute(target, event);

    if (route)
        return place(ichannel, routetarget(route), route->to, count, data);

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

        dispatch(&message);
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
            dispatch(&message);
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

void channel_pipe(unsigned int owner, struct event_pipe *pipe)
{

    unsigned int i;

    pipeowner = owner;
    pipeprev = pipe->prev;
    pipenext = (pipe->next) ? pipe->next : owner;
    nroutes = (pipe->nroutes < PIPE_ROUTES) ? pipe->nroutes : PIPE_ROUTES;

    for (i = 0; i < nroutes; i++)
        routes[i] = pipe->routes[i];

    routes[nroutes].event = EVENT_DATA;
    routes[nroutes].to = EVENT_DATA;
    routes[nroutes].target = 0;
    nroutes++;
    routes[nroutes].event = EVENT_ERROR;
    routes[nroutes].to = EVENT_ERROR;
    routes[nroutes].target = 0;
    nroutes++;

}

void channel_close(void)
{

    state = CHANNEL_STATE_CLOSED;

}

void channel_exit(unsigned int ichannel)
{

    struct event_route *route = findroute(EVENT_TERM);
    unsigned int term = (route) ? routetarget(route) : pipenext;
    unsigned int event = (route) ? route->to : EVENT_TERM;
    unsigned int i;

    if (term)
        place(ichannel, term, event, 0, 0);

    for (i = 0; i < nroutes; i++)
    {

        unsigned int target = routes[i].target;
        unsigned int j;

        if (!target || target == term || target == pipeowner)
            continue;

        for (j = 0; j < i; j++)
        {

            if (routes[j].target == target)
                break;

        }

        if (j == i)
            place(ichannel, target, EVENT_TERM, 0, 0);

    }

    if (pipeowner && (term != pipeowner || event != EVENT_TERM))
        place(ichannel, pipeowner, EVENT_TERM, 0, 0);

}

void channel_abort(unsigned int ichannel)
{

    char *error = "Process aborted\n";

    if (pipeowner)
        channel_send(ichannel, pipeowner, EVENT_ERROR, cstring_length(error), error);

    channel_exit(ichannel);

}

