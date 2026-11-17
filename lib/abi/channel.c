#include <fudge.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_MAILBOXES               (MESSAGE_PAGES / MESSAGE_SLOTS)
#define CHANNEL_QUEUE                   32
#define CHANNEL_STATE_OPENED            0
#define CHANNEL_STATE_CLOSING           1
#define CHANNEL_STATE_CLOSED            2

struct inbox
{

    struct message messages[CHANNEL_QUEUE];
    unsigned int head;
    unsigned int count;
    unsigned int done;
    unsigned int handed;
    unsigned int state;

};

static void (*listeners[CHANNEL_EVENTS])(struct message *message);

static struct inbox inboxes[CHANNEL_MAILBOXES];
static unsigned int owner;
static unsigned int status;
static struct message_pipe pipe;

static struct message_route *findroute(unsigned int event)
{

    unsigned int i;

    for (i = 0; i < pipe.nroutes && i < PIPE_ROUTES; i++)
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

    struct message_pipe *received = message_validate(MESSAGE_TYPE_MAIN, message->data, message->length);

    owner = message->source;

    if (received)
    {

        buffer_copy(&pipe, received, sizeof (struct message_pipe));

        if (pipe.nroutes > PIPE_ROUTES)
            pipe.nroutes = PIPE_ROUTES;

        if (pipe.nterms > PIPE_ROUTES)
            pipe.nterms = PIPE_ROUTES;

    }

    else
        message_initpipe(&pipe);

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

static unsigned int fill(unsigned int ichannel, struct inbox *inbox, unsigned int flags)
{

    struct message messages[MESSAGE_SLOTS];
    unsigned int status = call_pick(ichannel, messages, MESSAGE_SLOTS, inbox->done, flags);

    if (status != MESSAGE_FAILED)
        inbox->done = 0;

    if (status == MESSAGE_OK)
    {

        unsigned int i;

        for (i = 0; i < MESSAGE_SLOTS && messages[i].type; i++)
            buffer_copy(&inbox->messages[(inbox->head + inbox->count++) % CHANNEL_QUEUE], &messages[i], sizeof (struct message));

    }

    return status;

}

static unsigned int pull(void)
{

    unsigned int progress = 0;
    unsigned int i;

    for (i = 0; i < CHANNEL_MAILBOXES; i++)
    {

        struct inbox *inbox = &inboxes[i];

        if (inbox->done && call_pick(i, 0, 0, inbox->done, 0) != MESSAGE_FAILED)
        {

            inbox->done = 0;
            progress = 1;

        }

    }

    return progress;

}

static unsigned int take(unsigned int ichannel, struct message *message, unsigned int flags)
{

    struct inbox *inbox = &inboxes[ichannel];

    inbox->done |= inbox->handed;
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

    inboxes[ichannel].done |= 1 << message->slot;

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
            return message->type;

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

    if (message->type == MESSAGE_TYPE_MAIN && !ichannel)
        setpipe(message);

    if (message->type < CHANNEL_EVENTS && listeners[message->type])
        listeners[message->type](message);

    switch (message->type)
    {

    case MESSAGE_TYPE_TERM:
        if (inbox->state == CHANNEL_STATE_OPENED && frominput(ichannel, message->source))
            inbox->state = CHANNEL_STATE_CLOSING;

        break;

    case MESSAGE_TYPE_INTERRUPT:
        inbox->state = CHANNEL_STATE_CLOSED;

        break;

    }

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    if (event == MESSAGE_TYPE_ERROR && status == MESSAGE_STATUS_OK)
        status = MESSAGE_STATUS_ERROR;

    if (!ichannel && (target == owner || (pipe.prev && target == pipe.prev)))
    {

        struct message_route *route = findroute(event);

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

        return message.type;

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

        if (message->source == source && message->type == event)
        {

            hand(ichannel, message);

            return event;

        }

        finish(ichannel, message);

        if (message->source == source && message->type == MESSAGE_TYPE_STATUS)
            return 0;

    }

    return 0;

}

unsigned int channel_wait(unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct message message;

    return (channel_poll(ichannel, source, event, &message)) ? buffer_read(data, count, message.data, message.length, 0) : 0;

}

static unsigned int walk(unsigned int target, unsigned int parent, char *path, unsigned int length)
{

    char data[MESSAGE_SIZE];
    struct message_walkrequest *request = (struct message_walkrequest *)data;

    request->parent = parent;
    request->length = buffer_write(data, MESSAGE_SIZE, path, length, sizeof (struct message_walkrequest));

    if (channel_send(1, target, MESSAGE_TYPE_WALKREQUEST, sizeof (struct message_walkrequest) + request->length, data))
    {

        struct message_walkresponse response;

        return (channel_wait(1, target, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response)) ? response.id : 0;

    }

    return 0;

}

static unsigned int readvalue(unsigned int target, unsigned int id)
{

    struct message_readrequest request;

    request.id = id;
    request.offset = 0;
    request.count = 16;
    request.flags = 0;

    if (channel_send(1, target, MESSAGE_TYPE_READREQUEST, sizeof (struct message_readrequest), &request))
    {

        char data[MESSAGE_SIZE];
        struct message_readresponse *response;

        response = message_validate(MESSAGE_TYPE_READRESPONSE, data, channel_wait(1, target, MESSAGE_TYPE_READRESPONSE, MESSAGE_SIZE, data));

        if (!response || response->buffer || response->count > 16)
            return 0;

        return cstring_read_value((char *)(response + 1), response->count, 10);

    }

    return 0;

}

unsigned int channel_lookup(char *name)
{

    unsigned int length = cstring_length(name);
    unsigned int offset = buffer_eachbyte(name, length, ':', 0);
    unsigned int target = walk(MESSAGE_ROOT, 0, name, (offset) ? offset - 1 : length);

    if (target && offset && offset != length)
    {

        unsigned int directory;
        unsigned int file;

        directory = walk(target, 0, name + offset, length - offset);

        if (!directory)
            return 0;

        file = walk(target, directory, "inode", 5);

        return (file) ? readvalue(target, file) : 0;

    }

    return target;

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

void channel_setstatus(unsigned int code)
{

    status = code;

}

void channel_exit(unsigned int ichannel)
{

    unsigned int i;

    for (i = 0; i < pipe.nterms; i++)
        place(ichannel, resolve(pipe.terms[i].target), pipe.terms[i].to, 0, 0);

    if (owner)
    {

        struct message_status message;

        message.code = status;

        place(ichannel, owner, MESSAGE_TYPE_STATUS, sizeof (struct message_status), &message);

    }

}

static void abortdrain(void)
{

    struct message messages[MESSAGE_SLOTS];
    unsigned int i;

    for (i = 0; i < CHANNEL_MAILBOXES; i++)
    {

        unsigned int slots = 0;
        unsigned int j;

        if (call_pick(i, messages, MESSAGE_SLOTS, 0, 0) != MESSAGE_OK)
            continue;

        for (j = 0; j < MESSAGE_SLOTS && messages[j].type; j++)
            slots |= 1 << messages[j].slot;

        call_pick(i, 0, 0, slots, 0);

    }

}

static void abortplace(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    for (;;)
    {

        switch (call_place(ichannel, target, event, count, data))
        {

        case MESSAGE_RETRY:
            break;

        case MESSAGE_FULL:
            abortdrain();
            call_pick(ichannel, 0, 0, 0, MESSAGE_FLAG_BLOCK);

            break;

        default:
            return;

        }

    }

}

void channel_abort(unsigned int ichannel)
{

    unsigned int i;

    if (owner)
    {

        char *error = "Process aborted\n";
        struct message_route *route = (ichannel) ? 0 : findroute(MESSAGE_TYPE_ERROR);

        if (route)
            abortplace(ichannel, resolve(route->target), route->to, cstring_length(error), error);
        else
            abortplace(ichannel, owner, MESSAGE_TYPE_ERROR, cstring_length(error), error);

    }

    for (i = 0; i < pipe.nterms && i < PIPE_ROUTES; i++)
        abortplace(ichannel, resolve(pipe.terms[i].target), pipe.terms[i].to, 0, 0);

    if (owner)
    {

        struct message_status message;

        message.code = MESSAGE_STATUS_ABORTED;

        abortplace(ichannel, owner, MESSAGE_TYPE_STATUS, sizeof (struct message_status), &message);

    }

}

