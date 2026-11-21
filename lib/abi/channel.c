#include <fudge.h>
#include "call.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_BITS                    32
#define CHANNEL_MAILBOXES               (MESSAGE_PAGES / MESSAGE_SLOTS)
#define CHANNEL_STATE_OPENED            0
#define CHANNEL_STATE_CLOSING           1
#define CHANNEL_STATE_CLOSED            2
/* task mail is refused once a slot for replies is all that is left; the message being run holds one more */
#define CHANNEL_FULL                    (MESSAGE_SLOTS - 2)

struct inbox
{

    struct message messages[MESSAGE_SLOTS];
    volatile unsigned char *done;
    unsigned int index;
    unsigned int count;
    unsigned int handed;
    unsigned int state;

};

static void (*listeners[CHANNEL_EVENTS])(struct message *message);
static void (*raisedlisteners[CHANNEL_BITS])(void);
static unsigned int linked[CHANNEL_BITS];
static struct inbox inboxes[CHANNEL_MAILBOXES];
static unsigned int owner;
static unsigned int relieving;
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

static void locate(unsigned int ichannel, struct message *message)
{

    unsigned int page = ichannel * MESSAGE_SLOTS + message->slot;

    /* the done page follows the message pages: found from where any picked message's page lies */
    inboxes[ichannel].done = (volatile unsigned char *)message->data + MESSAGE_SIZE * (MESSAGE_PAGES - page) + ichannel * MESSAGE_SLOTS;

}

static void finish(unsigned int ichannel, struct message *message)
{

    inboxes[ichannel].done[message->slot] = 1;

}

static void fetchevents(unsigned int target);

static void runraised(void)
{

    unsigned int bits = call_collect();
    unsigned int i;

    for (i = 0; i < CHANNEL_BITS; i++)
    {

        if (!(bits & (1u << i)))
            continue;

        if (linked[i])
            fetchevents(linked[i]);
        else if (raisedlisteners[i])
            raisedlisteners[i]();

    }

}

static unsigned int fill(unsigned int ichannel, struct inbox *inbox, unsigned int flags)
{

    unsigned int status;
    unsigned int i;

    for (i = 0; inbox->index + i < inbox->count; i++)
        buffer_copy(&inbox->messages[i], &inbox->messages[inbox->index + i], sizeof (struct message));

    inbox->index = 0;
    inbox->count = i;

    if (inbox->count == MESSAGE_SLOTS)
        return MESSAGE_OK;

    status = call_pick(ichannel, &inbox->messages[inbox->count], MESSAGE_SLOTS - inbox->count, flags);

    /* raised bits have run their listeners: the caller decides whether to return or pick again */
    if (status == MESSAGE_RAISED)
        runraised();

    if (status == MESSAGE_OK)
    {

        locate(ichannel, &inbox->messages[inbox->count]);

        while (inbox->count < MESSAGE_SLOTS && inbox->messages[inbox->count].type)
            inbox->count++;

    }

    return status;

}

static unsigned int take(unsigned int ichannel, struct message *message, unsigned int flags)
{

    struct inbox *inbox = &inboxes[ichannel];

    if (inbox->handed)
        inbox->done[inbox->handed - 1] = 1;

    inbox->handed = 0;

    if (inbox->index == inbox->count)
    {

        unsigned int status = fill(ichannel, inbox, flags);

        if (status != MESSAGE_OK)
            return status;

    }

    buffer_copy(message, &inbox->messages[inbox->index++], sizeof (struct message));

    return MESSAGE_OK;

}

static void hand(unsigned int ichannel, struct message *message)
{

    inboxes[ichannel].handed = message->slot + 1;

}

static unsigned int canpick(unsigned int ichannel)
{

    return ichannel < CHANNEL_MAILBOXES && inboxes[ichannel].state != CHANNEL_STATE_CLOSED;

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

static void relieve(void)
{

    struct inbox *inbox = &inboxes[0];
    struct message message;

    /* someone may be retrying a send into our full channel 0: run its oldest message, but never from inside another one */
    if (relieving || inbox->handed || !canpick(0))
        return;

    fill(0, inbox, 0);

    if (inbox->count - inbox->index >= CHANNEL_FULL && take(0, &message, 0) == MESSAGE_OK)
    {

        relieving = 1;

        dispatch(0, &message);
        finish(0, &message);

        relieving = 0;

    }

}

static unsigned int pick(unsigned int ichannel, struct message *message)
{

    while (canpick(ichannel))
    {

        unsigned int status = take(ichannel, message, MESSAGE_FLAG_BLOCK | MESSAGE_FLAG_RAISED);

        switch (status)
        {

        case MESSAGE_RETRY:
            if (ichannel)
                relieve();

            continue;

        case MESSAGE_RAISED:
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

    unsigned int status;

    while ((status = call_place(ichannel, target, event, count, data)) == MESSAGE_RETRY)
        relieve();

    return (status == MESSAGE_OK) ? event : 0;

}

static unsigned int link(unsigned int ichannel, unsigned int target)
{

    struct message_link message;
    unsigned int i;

    /* bits from the top are the library's: one per linked service, which raises it and is asked for its new events */
    for (i = CHANNEL_BITS; i-- > 0;)
    {

        if (!linked[i] && !raisedlisteners[i])
        {

            linked[i] = target;
            message.bit = i;

            if (place(ichannel, target, MESSAGE_TYPE_LINK, sizeof (struct message_link), &message))
                return MESSAGE_TYPE_LINK;

            linked[i] = 0;

            return 0;

        }

    }

    return place(ichannel, target, MESSAGE_TYPE_LINK, 0, 0);

}

static unsigned int unlink(unsigned int ichannel, unsigned int target)
{

    unsigned int i;

    for (i = 0; i < CHANNEL_BITS; i++)
    {

        if (linked[i] == target)
            linked[i] = 0;

    }

    return place(ichannel, target, MESSAGE_TYPE_UNLINK, 0, 0);

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    if (event == MESSAGE_TYPE_LINK && !count)
        return link(ichannel, resolve(target));

    if (event == MESSAGE_TYPE_UNLINK && !count)
        return unlink(ichannel, resolve(target));

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

    while (canpick(ichannel))
    {

        switch (take(ichannel, &message, MESSAGE_FLAG_BLOCK | MESSAGE_FLAG_RAISED))
        {

        case MESSAGE_OK:
            dispatch(ichannel, &message);
            finish(ichannel, &message);

            return message.type;

        /* raised listeners ran: return, so the caller does what follows a message (the wm draws) */
        case MESSAGE_RAISED:
            return CHANNEL_EVENTS;

        case MESSAGE_RETRY:
            if (ichannel)
                relieve();

            break;

        default:
            return 0;

        }

    }

    return 0;

}

unsigned int channel_drain(unsigned int ichannel)
{

    struct message message;

    while (canpick(ichannel))
    {

        switch (take(ichannel, &message, MESSAGE_FLAG_RAISED))
        {

        case MESSAGE_OK:
            dispatch(ichannel, &message);
            finish(ichannel, &message);

            break;

        case MESSAGE_RAISED:
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

unsigned int channel_call(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data, unsigned int replyevent, unsigned int replycount, void *replydata)
{

    char buffer[MESSAGE_SIZE];
    struct message reply;
    unsigned int status;

    message_init(&reply, 0, 0, MESSAGE_SIZE, buffer);

    while ((status = call_call(ichannel, target, event, count, data, &reply)) == MESSAGE_RETRY)
        relieve();

    /* a kernel service answers straight into the reply, which listeners see like any awaited answer; a task, or a service answering later, answers with mail */
    if (status == MESSAGE_OK)
    {

        dispatch(ichannel, &reply);

        return (reply.type == replyevent) ? buffer_read(replydata, replycount, reply.data, reply.length, 0) : 0;

    }

    return (status == MESSAGE_PENDING) ? channel_wait(ichannel, target, replyevent, replycount, replydata) : 0;

}

static void fetchevents(unsigned int target)
{

    char data[MESSAGE_SIZE];
    struct message_eventsresponse *response = (struct message_eventsresponse *)data;
    unsigned int offset = sizeof (struct message_eventsresponse);
    unsigned int count;
    unsigned int i;

    count = channel_call(1, target, MESSAGE_TYPE_EVENTSREQUEST, 0, 0, MESSAGE_TYPE_EVENTSRESPONSE, MESSAGE_SIZE, data);

    for (i = 0; count >= offset && i < response->count && offset + sizeof (struct message_eventsentry) <= count; i++)
    {

        struct message_eventsentry *entry = (struct message_eventsentry *)(data + offset);
        struct message message;

        offset += sizeof (struct message_eventsentry);

        if (entry->length > count - offset)
            break;

        message_init(&message, entry->event, target, entry->length, data + offset);

        if (entry->event < CHANNEL_EVENTS && listeners[entry->event])
            listeners[entry->event](&message);

        offset += (entry->length + 3) & ~3u;

    }

}

static unsigned int walk(unsigned int target, unsigned int parent, char *path, unsigned int length)
{

    struct message_walkresponse response;
    char data[MESSAGE_SIZE];
    struct message_walkrequest *request = (struct message_walkrequest *)data;

    request->parent = parent;
    request->length = buffer_write(data, MESSAGE_SIZE, path, length, sizeof (struct message_walkrequest));

    return (channel_call(1, target, MESSAGE_TYPE_WALKREQUEST, sizeof (struct message_walkrequest) + request->length, data, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response)) ? response.id : 0;

}

static unsigned int readvalue(unsigned int target, unsigned int id)
{

    struct message_readrequest request;
    char data[MESSAGE_SIZE];
    struct message_readresponse *response;

    request.id = id;
    request.offset = 0;
    request.count = 16;
    request.flags = 0;
    response = message_validate(MESSAGE_TYPE_READRESPONSE, data, channel_call(1, target, MESSAGE_TYPE_READREQUEST, sizeof (struct message_readrequest), &request, MESSAGE_TYPE_READRESPONSE, MESSAGE_SIZE, data));

    if (!response || response->buffer || response->count > 16)
        return 0;

    return cstring_read_value((char *)(response + 1), response->count, 10);

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

void channel_bindraised(unsigned int bit, void (*callback)(void))
{

    if (bit < CHANNEL_BITS)
        raisedlisteners[bit] = callback;

}

unsigned int channel_raise(unsigned int target, unsigned int bit)
{

    return (bit < CHANNEL_BITS) ? call_raise(resolve(target), 1u << bit) == MESSAGE_OK : 0;

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

    call_collect();

    for (i = 0; i < CHANNEL_MAILBOXES; i++)
    {

        unsigned int j;

        if (call_pick(i, messages, MESSAGE_SLOTS, 0) != MESSAGE_OK)
            continue;

        locate(i, &messages[0]);

        for (j = 0; j < MESSAGE_SLOTS && messages[j].type; j++)
            finish(i, &messages[j]);

    }

}

static void abortplace(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    do
    {

        abortdrain();

    } while (call_place(ichannel, target, event, count, data) == MESSAGE_RETRY);

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

