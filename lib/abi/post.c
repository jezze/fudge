#include <fudge.h>
#include "call.h"
#include "channel.h"

#define POST_OUTBOXES                   8
#define POST_SIZE                       0x1000

struct outbox
{

    unsigned int ichannel;
    unsigned int target;
    unsigned int length;
    char data[POST_SIZE];

};

static struct outbox outboxes[POST_OUTBOXES];

static struct outbox *findoutbox(unsigned int ichannel, unsigned int target, unsigned int create)
{

    struct outbox *empty = 0;
    unsigned int i;

    for (i = 0; i < POST_OUTBOXES; i++)
    {

        struct outbox *outbox = &outboxes[i];

        if (outbox->length && outbox->ichannel == ichannel && outbox->target == target)
            return outbox;

        if (!outbox->length && !empty)
            empty = outbox;

    }

    if (create && empty)
    {

        empty->ichannel = ichannel;
        empty->target = target;

    }

    return (create) ? empty : 0;

}

static void flush(struct outbox *outbox)
{

    while (outbox->length)
    {

        struct message_eventsentry *entry = (struct message_eventsentry *)outbox->data;
        unsigned int size = sizeof (struct message_eventsentry) + ((entry->length + 3) & ~3u);
        unsigned int i;

        if (channel_trysend(outbox->ichannel, outbox->target, entry->event, entry->length, entry + 1) == MESSAGE_RETRY)
            break;

        outbox->length -= size;

        /* the rest moves to the front; the regions overlap, so byte by byte from the start */
        for (i = 0; i < outbox->length; i++)
            outbox->data[i] = outbox->data[i + size];

    }

}

static void onroom(void)
{

    unsigned int i;

    for (i = 0; i < POST_OUTBOXES; i++)
    {

        if (outboxes[i].length)
            flush(&outboxes[i]);

    }

}

unsigned int channel_post(unsigned int ichannel, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct outbox *outbox = findoutbox(ichannel, target, 0);
    unsigned int size = sizeof (struct message_eventsentry) + ((count + 3) & ~3u);
    struct message_eventsentry *entry;

    channel_bindraised(MESSAGE_BIT_ROOM, onroom);

    if (outbox)
        flush(outbox);

    /* never waits for the target: what it has no room for waits here, in order, until the kernel says it has room */
    if (!outbox || !outbox->length)
    {

        unsigned int status = channel_trysend(ichannel, target, event, count, data);

        if (status != MESSAGE_RETRY)
            return status == MESSAGE_OK;

    }

    outbox = findoutbox(ichannel, target, 1);

    if (!outbox || outbox->length + size > POST_SIZE)
        return 0;

    entry = (struct message_eventsentry *)(outbox->data + outbox->length);
    entry->event = event;
    entry->length = count;

    buffer_copy(entry + 1, data, count);

    outbox->length += size;

    return 1;

}
