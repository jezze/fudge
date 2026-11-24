#include <fudge.h>
#include "call.h"
#include "page.h"
#include "channel.h"

#define CHANNEL_EVENTS                  256
#define CHANNEL_STATE_OPENED            0
#define CHANNEL_STATE_CLOSING           1
#define CHANNEL_STATE_CLOSED            2

struct inbox
{

    struct message_box boxes[MESSAGE_SLOTS];
    struct message_box held;
    unsigned int index;
    unsigned int count;
    unsigned int used;
    unsigned int state;

};

static void (*listeners[CHANNEL_EVENTS])(struct message *message);
static struct inbox inboxes[MESSAGE_CHANNELS];
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

/* where a message to the owner or the previous stage really goes: to = the event sent, target = the task */
static struct message_route route(unsigned int ichannel, unsigned int target, unsigned int event)
{

    struct message_route result;

    result.event = event;
    result.to = event;
    result.target = target;

    if (!ichannel && (target == owner || (pipe.prev && target == pipe.prev)))
    {

        struct message_route *found = findroute(event);

        if (found)
        {

            result.to = found->to;
            result.target = resolve(found->target);

        }

    }

    return result;

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

/* mail was copied out when picked, so only a page given with a message is left to give back */
static void finish(struct message *message)
{

    if (message->page)
        page_free(message->data);

}

/* a box is moved or copied whole: mail data is its own payload, a page stays where it is */
static void copybox(struct message_box *to, struct message_box *from)
{

    buffer_copy(to, from, sizeof (struct message_box));

    if (from->message.data == from->payload)
        to->message.data = to->payload;

}

static unsigned int fill(unsigned int ichannel, struct inbox *inbox, unsigned int flags)
{

    unsigned int status;
    unsigned int i;

    for (i = 0; inbox->index + i < inbox->count; i++)
        copybox(&inbox->boxes[i], &inbox->boxes[inbox->index + i]);

    inbox->index = 0;
    inbox->count = i;

    if (inbox->count == MESSAGE_SLOTS)
        return MESSAGE_OK;

    status = call_pick(ichannel, &inbox->boxes[inbox->count], MESSAGE_SLOTS - inbox->count, flags);

    if (status == MESSAGE_OK)
    {

        while (inbox->count < MESSAGE_SLOTS && inbox->boxes[inbox->count].message.type)
            inbox->count++;

    }

    return status;

}

/* pages come in the mailbox with the mail, in the order all of it was sent */
/* into a box of the caller's: a handler may pick more on the same channel before it is done with this one */
static unsigned int take(unsigned int ichannel, struct message_box *box, unsigned int flags)
{

    struct inbox *inbox = &inboxes[ichannel];

    inbox->used = 1;

    channel_done(ichannel);

    if (inbox->index == inbox->count)
    {

        unsigned int status = fill(ichannel, inbox, flags);

        if (status != MESSAGE_OK)
            return status;

    }

    copybox(box, &inbox->boxes[inbox->index++]);

    return MESSAGE_OK;

}

/* the message is the caller's until its next pick on the channel, and so is a page given with it */
static struct message *hand(unsigned int ichannel, struct message_box *box)
{

    struct inbox *inbox = &inboxes[ichannel];

    copybox(&inbox->held, box);

    return &inbox->held.message;

}

static unsigned int canpick(unsigned int ichannel)
{

    return ichannel < MESSAGE_CHANNELS && inboxes[ichannel].state != CHANNEL_STATE_CLOSED;

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

/* while a send waits, queued mail is moved into the inbox on every channel, which frees its slots for whoever waits on this task; nothing is handled out of turn */
static void relieve(void)
{

    unsigned int i;

    /* only channels already in use: a pick on any other would take a mailbox from the pool for nothing */
    for (i = 0; i < MESSAGE_CHANNELS; i++)
    {

        if (inboxes[i].used && canpick(i))
            fill(i, &inboxes[i], 0);

    }

}

static unsigned int pick(unsigned int ichannel, struct message_box *box)
{

    while (canpick(ichannel))
    {

        unsigned int status = take(ichannel, box, MESSAGE_FLAG_BLOCK);

        switch (status)
        {

        case MESSAGE_RETRY:
            if (ichannel)
                relieve();

            continue;

        case MESSAGE_OK:
            return box->message.type;

        case MESSAGE_FAILED:
            return 0;

        }

    }

    return 0;

}

/* the payload is the data given, or the page given */
static unsigned int deliver(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data, unsigned int page)
{

    unsigned int status;

    if (ichannel < MESSAGE_CHANNELS)
        inboxes[ichannel].used = 1;

    while ((status = call_place(ichannel, target, event, count, data, page)) == MESSAGE_RETRY)
        relieve();

    return status == MESSAGE_OK;

}

/* the payload goes in a new page; the page is the receiver's to free once read */
static unsigned int placepage(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    void *page;

    if (count > PAGE_SIZE)
        return 0;

    page = page_alloc();

    buffer_copy(page, data, count);

    if (deliver(ichannel, target, event, count, 0, page_number(page)))
        return event;

    page_free(page);

    return 0;

}

/* control is small and never cut short: one too large fails, reported where this task reports its errors */
unsigned int channel_toolong(void)
{

    char *error = "Control message too long\n";

    if (owner)
        channel_send(0, owner, MESSAGE_TYPE_ERROR, cstring_length(error), error);

    return 0;

}

static unsigned int place(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    if (message_isdata(event))
        return placepage(ichannel, target, event, count, data);

    if (count > MESSAGE_CONTROLSIZE)
        return channel_toolong();

    return (deliver(ichannel, target, event, count, data, 0)) ? event : 0;

}

unsigned int channel_send(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct message_route to;
    struct message_error error;

    /* an error is a fixed control message: the text given, up to its end, goes in the struct */
    if (event == MESSAGE_TYPE_ERROR)
    {

        unsigned int length = buffer_findbyte(data, count, '\0');

        if (length >= MESSAGE_TEXTSIZE)
            return channel_toolong();

        buffer_clear(&error, sizeof (struct message_error));
        buffer_copy(error.text, data, length);

        count = sizeof (struct message_error);
        data = &error;

        if (status == MESSAGE_STATUS_OK)
            status = MESSAGE_STATUS_ERROR;

    }

    to = route(ichannel, target, event);

    return place(ichannel, to.target, to.to, count, data);

}

/* data that came in a page goes on in that page; the message no longer holds it */
unsigned int channel_forward(unsigned int ichannel, unsigned int target, unsigned int event, struct message *message)
{

    struct message_route to = route(ichannel, target, event);

    if (message->page && message_isdata(to.to))
    {

        if (!deliver(ichannel, to.target, to.to, message->length, 0, message->page))
            return 0;

        message->page = 0;

        return to.to;

    }

    return channel_send(ichannel, target, event, message->length, message->data);

}

unsigned int channel_sendoption(unsigned int ichannel, unsigned int target, char *key, char *value)
{

    struct message_option option;

    if (cstring_length(key) >= MESSAGE_KEYSIZE || cstring_length(value) >= MESSAGE_VALUESIZE)
        return channel_toolong();

    buffer_clear(&option, sizeof (struct message_option));
    buffer_copy(option.key, key, cstring_length(key));
    buffer_copy(option.value, value, cstring_length(value));

    return channel_send(ichannel, target, MESSAGE_TYPE_OPTION, sizeof (struct message_option), &option);

}

unsigned int channel_sendid(unsigned int ichannel, unsigned int target, unsigned int event, char *id)
{

    struct message_id message;

    if (cstring_length(id) >= MESSAGE_IDSIZE)
        return channel_toolong();

    buffer_clear(&message, sizeof (struct message_id));
    buffer_copy(message.id, id, cstring_length(id));

    return channel_send(ichannel, target, event, sizeof (struct message_id), &message);

}

unsigned int channel_sendpath(unsigned int ichannel, unsigned int target, char *path)
{

    struct message_path message;

    if (cstring_length(path) >= MESSAGE_PATHSIZE)
        return channel_toolong();

    buffer_clear(&message, sizeof (struct message_path));
    buffer_copy(message.path, path, cstring_length(path));

    return channel_send(ichannel, target, MESSAGE_TYPE_PATH, sizeof (struct message_path), &message);

}

unsigned int channel_send_fmtv(unsigned int ichannel, unsigned int target, unsigned int event, char *fmt, void **args)
{

    char buffer[MESSAGE_SIZE];

    return channel_send(ichannel, target, event, cstring_write_fmtv(buffer, MESSAGE_SIZE, 0, fmt, args), buffer);

}

struct message *channel_pick(unsigned int ichannel)
{

    struct message_box box;

    return (pick(ichannel, &box)) ? hand(ichannel, &box) : 0;

}

unsigned int channel_process(unsigned int ichannel)
{

    struct message_box box;

    if (pick(ichannel, &box))
    {

        dispatch(ichannel, &box.message);
        finish(&box.message);

        return box.message.type;

    }

    return 0;

}

unsigned int channel_drain(unsigned int ichannel)
{

    struct message_box box;

    while (canpick(ichannel))
    {

        switch (take(ichannel, &box, 0))
        {

        case MESSAGE_OK:
            dispatch(ichannel, &box.message);
            finish(&box.message);

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

    while (ichannel < MESSAGE_CHANNELS && inboxes[ichannel].state == CHANNEL_STATE_OPENED && channel_process(ichannel));

}

static struct message *poll(unsigned int ichannel, unsigned int source, unsigned int event, unsigned int pages)
{

    struct message_box box;

    while (pick(ichannel, &box))
    {

        /* a reply given in a page is the caller's, not data for the listeners */
        if (pages && box.message.source == source && box.message.page)
            return hand(ichannel, &box);

        dispatch(ichannel, &box.message);

        if (!pages && box.message.source == source && box.message.type == event)
            return hand(ichannel, &box);

        finish(&box.message);

        if (box.message.source == source && box.message.type == MESSAGE_TYPE_STATUS)
            return 0;

    }

    return 0;

}

struct message *channel_poll(unsigned int ichannel, unsigned int source, unsigned int event)
{

    return poll(ichannel, source, event, 0);

}

/* a reply comes in a page from the source; mail from the source meanwhile goes to the listeners as usual */
struct message *channel_pollreply(unsigned int ichannel, unsigned int source)
{

    return poll(ichannel, source, 0, 1);

}

unsigned int channel_wait(unsigned int ichannel, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct message *message = channel_poll(ichannel, source, event);

    return (message) ? buffer_read(data, count, message->data, message->length, 0) : 0;

}

static unsigned int walk(unsigned int target, unsigned int parent, char *path, unsigned int length)
{

    struct {struct message_walkrequest request; char path[MESSAGE_PATHSIZE];} message;

    if (length > MESSAGE_PATHSIZE)
        return channel_toolong();

    buffer_clear(&message, sizeof (message));
    buffer_copy(message.path, path, length);

    message.request.parent = parent;
    message.request.length = length;

    if (channel_send(1, target, MESSAGE_TYPE_WALKREQUEST, sizeof (message), &message))
    {

        struct message_walkresponse response;

        return (channel_wait(1, target, MESSAGE_TYPE_WALKRESPONSE, sizeof (struct message_walkresponse), &response)) ? response.id : 0;

    }

    return 0;

}

static unsigned int readvalue(unsigned int target, unsigned int id)
{

    struct message_readrequest request;
    struct message *message;
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

    if (channel_send(1, target, MESSAGE_TYPE_READREQUEST, sizeof (struct message_readrequest), &request) && (message = channel_pollreply(1, target)))
    {

        if (message->length <= 16)
            value = cstring_read_value(message->data, message->length, 10);

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

    struct message *message = &inboxes[ichannel].held.message;

    finish(message);

    message->page = 0;

}

void channel_bind(unsigned int event, void (*callback)(struct message *message))
{

    listeners[event] = callback;

}

void channel_close(unsigned int ichannel)
{

    if (ichannel < MESSAGE_CHANNELS)
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

    struct message_box boxes[MESSAGE_SLOTS];
    unsigned int i;

    for (i = 0; i < MESSAGE_CHANNELS; i++)
    {

        unsigned int j;

        if (!inboxes[i].used || call_pick(i, boxes, MESSAGE_SLOTS, 0) != MESSAGE_OK)
            continue;

        for (j = 0; j < MESSAGE_SLOTS && boxes[j].message.type; j++)
            finish(&boxes[j].message);

    }

}

static void abortplace(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    do
    {

        abortdrain();

    } while (call_place(ichannel, target, event, count, data, 0) == MESSAGE_RETRY);

}

void channel_abort(unsigned int ichannel)
{

    unsigned int i;

    if (owner)
    {

        struct message_error error;
        struct message_route to = route(ichannel, owner, MESSAGE_TYPE_ERROR);

        buffer_clear(&error, sizeof (struct message_error));
        buffer_copy(error.text, "Process aborted\n", 16);
        abortplace(ichannel, to.target, to.to, sizeof (struct message_error), &error);

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

