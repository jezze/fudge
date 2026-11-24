#include <fudge.h>
#include "call.h"
#include "page.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_MAILBOXES               (MESSAGE_PAGES / MESSAGE_SLOTS)
#define CHANNEL_STATE_OPENED            0
#define CHANNEL_STATE_CLOSING           1
#define CHANNEL_STATE_CLOSED            2
/* task mail is refused once a slot for replies is all that is left; the message being run holds one more */
#define CHANNEL_FULL                    (MESSAGE_SLOTS - 2)
/* data that came in a page has no mailbox slot; these slots say so, and whether the page is still to be freed */
#define CHANNEL_SLOTPAGE                MESSAGE_SLOTS
#define CHANNEL_SLOTKEPT                (MESSAGE_SLOTS + 1)
/* handlers may run inside handlers, as relieve does */
#define CHANNEL_ACTIVE                  8

struct inbox
{

    struct message messages[MESSAGE_SLOTS];
    volatile unsigned char *done;
    unsigned int index;
    unsigned int count;
    unsigned int handed;
    void *handedpage;
    unsigned int state;

};

static void (*listeners[CHANNEL_EVENTS])(struct message *message);
static struct inbox inboxes[CHANNEL_MAILBOXES];
static unsigned int owner;
static unsigned int started;
static struct message *active[CHANNEL_ACTIVE];
static unsigned int nactive;
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
    started = 1;

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

    /* the done page follows the slots: found from where any picked message's slot lies */
    inboxes[ichannel].done = (volatile unsigned char *)message->data + MESSAGE_SLOTSIZE * (MESSAGE_PAGES - page) + ichannel * MESSAGE_SLOTS;

}

static void finish(unsigned int ichannel, struct message *message)
{

    if (message->slot < MESSAGE_SLOTS)
        inboxes[ichannel].done[message->slot] = 1;
    else if (message->slot == CHANNEL_SLOTPAGE)
        page_free(message->data);

}

static unsigned int isend(struct message *message)
{

    return message->type == MESSAGE_TYPE_TERM || message->type == MESSAGE_TYPE_STATUS;

}

static unsigned int takepage(unsigned int ichannel, struct message *message)
{

    struct page_data input = page_trytake(ichannel);

    if (!input.data)
        return 0;

    message_init(message, MESSAGE_TYPE_DATA, input.source, input.count, input.data);

    message->slot = CHANNEL_SLOTPAGE;

    return 1;

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

    if (status == MESSAGE_OK)
    {

        locate(ichannel, &inbox->messages[inbox->count]);

        while (inbox->count < MESSAGE_SLOTS && inbox->messages[inbox->count].type)
            inbox->count++;

    }

    return status;

}

/* a program gets no data on channel 0 before its MAIN; mail comes before pages, but TERM and STATUS wait for the pages given before them */
static unsigned int take(unsigned int ichannel, struct message *message, unsigned int flags)
{

    struct inbox *inbox = &inboxes[ichannel];
    unsigned int status = MESSAGE_OK;

    channel_done(ichannel);

    for (;;)
    {

        if (inbox->index == inbox->count)
            status = fill(ichannel, inbox, 0);

        if (status == MESSAGE_FAILED)
            return status;

        if ((ichannel || started) && (inbox->index == inbox->count || isend(&inbox->messages[inbox->index])) && takepage(ichannel, message))
            return MESSAGE_OK;

        if (inbox->index < inbox->count)
            break;

        /* mail that came while waiting may be a TERM sent after pages that came with it, so the pages are looked for again */
        status = fill(ichannel, inbox, flags);

        if (status != MESSAGE_OK)
            return status;

    }

    buffer_copy(message, &inbox->messages[inbox->index++], sizeof (struct message));

    return MESSAGE_OK;

}

static void hand(unsigned int ichannel, struct message *message)
{

    if (message->slot < MESSAGE_SLOTS)
        inboxes[ichannel].handed = message->slot + 1;
    else if (message->slot == CHANNEL_SLOTPAGE)
        inboxes[ichannel].handedpage = message->data;

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

    if (nactive < CHANNEL_ACTIVE)
        active[nactive++] = message;

    if (message->type < CHANNEL_EVENTS && listeners[message->type])
        listeners[message->type](message);

    if (nactive && active[nactive - 1] == message)
        nactive--;

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
    if (relieving || inbox->handed || inbox->handedpage || !canpick(0))
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

        unsigned int status = take(ichannel, message, MESSAGE_FLAG_BLOCK);

        switch (status)
        {

        case MESSAGE_RETRY:
            if (ichannel)
                relieve();

            continue;

        case MESSAGE_OK:
            return message->type;

        case MESSAGE_FAILED:
            return 0;

        }

    }

    return 0;

}

/* the data is a page this task was handed and still holds: from here it is not freed, as it is about to be given on */
static unsigned int handover(void *data)
{

    unsigned int i;

    for (i = nactive; i > 0; i--)
    {

        struct message *message = active[i - 1];

        if (message->slot == CHANNEL_SLOTPAGE && message->data == data)
        {

            message->slot = CHANNEL_SLOTKEPT;

            return 1;

        }

    }

    for (i = 0; i < CHANNEL_MAILBOXES; i++)
    {

        if (inboxes[i].handedpage == data)
        {

            inboxes[i].handedpage = 0;

            return 1;

        }

    }

    return 0;

}

/* pipe data goes in a page, the one it came in if the handler sends that on; the page is the receiver's to free once read */
static unsigned int placepage(unsigned int target, unsigned int count, void *data)
{

    void *page;

    if (count > PAGE_SIZE)
        return 0;

    if (handover(data))
        return (page_give(data, target, count)) ? MESSAGE_TYPE_DATA : 0;

    page = page_alloc();

    buffer_copy(page, data, count);

    if (page_give(page, target, count))
        return MESSAGE_TYPE_DATA;

    page_free(page);

    return 0;

}

static unsigned int place(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    unsigned int status;

    while ((status = call_place(ichannel, target, event, count, data)) == MESSAGE_RETRY)
        relieve();

    return (status == MESSAGE_OK) ? event : 0;

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    if (event == CHANNEL_PAGE)
        return placepage(target, count, data);

    if (event == MESSAGE_TYPE_ERROR && status == MESSAGE_STATUS_OK)
        status = MESSAGE_STATUS_ERROR;

    if (!ichannel && (target == owner || (pipe.prev && target == pipe.prev)))
    {

        struct message_route *route = findroute(event);

        if (route && route->to == MESSAGE_TYPE_DATA)
            return placepage(resolve(route->target), count, data);

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

static unsigned int poll(unsigned int ichannel, unsigned int source, unsigned int event, unsigned int pages, struct message *message)
{

    while (pick(ichannel, message))
    {

        /* a reply given in a page is the caller's, not data for the listeners */
        if (pages && message->source == source && message->slot == CHANNEL_SLOTPAGE)
        {

            hand(ichannel, message);

            return MESSAGE_TYPE_DATA;

        }

        dispatch(ichannel, message);

        if (!pages && message->source == source && message->type == event)
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

unsigned int channel_poll(unsigned int ichannel, unsigned int source, unsigned int event, struct message *message)
{

    return poll(ichannel, source, event, 0, message);

}

/* a reply comes in a page from the source; mail from the source meanwhile goes to the listeners as usual */
unsigned int channel_pollreply(unsigned int ichannel, unsigned int source, struct message *message)
{

    return poll(ichannel, source, 0, 1, message);

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
    struct message message;
    void *page = page_alloc();
    unsigned int value = 0;

    request.id = id;
    request.offset = 0;
    request.count = 16;
    request.flags = 0;
    request.page = page_hand(page, target);

    if (!request.page)
    {

        page_free(page);

        return 0;

    }

    if (channel_send(1, target, MESSAGE_TYPE_READREQUEST, sizeof (struct message_readrequest), &request) && channel_pollreply(1, target, &message) == MESSAGE_TYPE_DATA)
    {

        if (message.length <= 16)
            value = cstring_read_value(message.data, message.length, 10);

        channel_done(1);

        return value;

    }

    page_free(page);

    return value;

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

/* the message last picked or polled on this channel is finished with now, rather than at the next pick */
void channel_done(unsigned int ichannel)
{

    struct inbox *inbox = &inboxes[ichannel];

    if (inbox->handed)
        inbox->done[inbox->handed - 1] = 1;

    if (inbox->handedpage)
        page_free(inbox->handedpage);

    inbox->handed = 0;
    inbox->handedpage = 0;

}

unsigned int channel_keep(struct message *message)
{

    unsigned int i;

    if (message->slot != CHANNEL_SLOTPAGE)
        return 0;

    message->slot = CHANNEL_SLOTKEPT;

    for (i = 0; i < CHANNEL_MAILBOXES; i++)
    {

        if (inboxes[i].handedpage == message->data)
            inboxes[i].handedpage = 0;

    }

    return 1;

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

