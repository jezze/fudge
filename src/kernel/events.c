#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "pool.h"
#include "service.h"
#include "mailbox.h"
#include "kernel.h"
#include "events.h"

static struct events_subscriber *findsubscriber(struct events *events, unsigned int source)
{

    struct mailbox *mailbox = pool_getnodemailbox(source);
    unsigned int i;

    for (i = 0; mailbox && i < EVENTS_SUBSCRIBERS; i++)
    {

        struct mailbox *subscriber = pool_getnodemailbox(events->subscribers[i].target);

        if (subscriber && subscriber->itask == mailbox->itask)
            return &events->subscribers[i];

    }

    return 0;

}

unsigned int events_record(struct events *events, unsigned int event, unsigned int count, void *data)
{

    char *bytes = data;
    unsigned int offset = 0;
    unsigned int i;

    spinlock_acquire(&events->spinlock);

    /* only byte streams (console data) are longer than an entry; they continue in the next ones */
    do
    {

        struct events_entry *entry = &events->ring[events->head % EVENTS_RING];

        entry->event = event;
        entry->length = (count - offset > EVENTS_DATASIZE) ? EVENTS_DATASIZE : count - offset;

        buffer_copy(entry->data, bytes + offset, entry->length);

        offset += entry->length;
        events->head++;

    } while (offset < count);

    spinlock_release(&events->spinlock);

    for (i = 0; i < EVENTS_SUBSCRIBERS; i++)
    {

        struct mailbox *mailbox = pool_getnodemailbox(events->subscribers[i].target);

        if (mailbox)
            kernel_raise(mailbox->itask, 1u << events->subscribers[i].bit);

    }

    return 1;

}

unsigned int events_subscribe(struct events *events, unsigned int source, unsigned int bit)
{

    unsigned int i;

    spinlock_acquire(&events->spinlock);

    for (i = 0; i < EVENTS_SUBSCRIBERS; i++)
    {

        struct events_subscriber *subscriber = &events->subscribers[i];

        /* a subscriber whose task is gone has a renewed inode that no longer finds a mailbox */
        if (!pool_getnodemailbox(subscriber->target))
        {

            subscriber->target = source;
            subscriber->bit = bit;
            subscriber->next = events->head;

            break;

        }

    }

    spinlock_release(&events->spinlock);

    return i < EVENTS_SUBSCRIBERS;

}

unsigned int events_unsubscribe(struct events *events, unsigned int source)
{

    struct events_subscriber *subscriber;

    spinlock_acquire(&events->spinlock);

    subscriber = findsubscriber(events, source);

    if (subscriber)
        subscriber->target = 0;

    spinlock_release(&events->spinlock);

    return subscriber != 0;

}

unsigned int events_send(struct events *events, unsigned int inode, unsigned int source)
{

    char data[MESSAGE_SIZE];
    struct message_eventsresponse *response = (struct message_eventsresponse *)data;
    unsigned int offset = sizeof (struct message_eventsresponse);
    struct events_subscriber *subscriber;

    response->count = 0;
    response->lost = 0;

    spinlock_acquire(&events->spinlock);

    subscriber = findsubscriber(events, source);

    if (subscriber)
    {

        if (events->head - subscriber->next > EVENTS_RING)
        {

            response->lost = events->head - subscriber->next - EVENTS_RING;
            subscriber->next = events->head - EVENTS_RING;

        }

        while (subscriber->next != events->head)
        {

            struct events_entry *entry = &events->ring[subscriber->next % EVENTS_RING];
            struct message_eventsentry *header = (struct message_eventsentry *)(data + offset);

            header->event = entry->event;
            header->length = entry->length;
            offset += sizeof (struct message_eventsentry);

            buffer_copy(data + offset, entry->data, entry->length);

            offset += (entry->length + 3) & ~3u;
            response->count++;
            subscriber->next++;

        }

    }

    spinlock_release(&events->spinlock);

    return kernel_placereply(source, inode, MESSAGE_TYPE_EVENTSRESPONSE, offset, data);

}

void events_init(struct events *events)
{

    unsigned int i;

    spinlock_init(&events->spinlock);

    events->head = 0;

    for (i = 0; i < EVENTS_SUBSCRIBERS; i++)
        events->subscribers[i].target = 0;

}
