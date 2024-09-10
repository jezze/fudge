#include <fudge.h>
#include "resource.h"
#include "mailbox.h"

unsigned int mailbox_pick(struct mailbox *mailbox, struct message *message, unsigned int count, void *data)
{

    unsigned int length = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (ring_count(&mailbox->ring))
    {

        unsigned int datasize;

        length += ring_read_all(&mailbox->ring, message, sizeof (struct message));

        datasize = message_datasize(message);

        if (datasize <= count)
        {

            length += ring_read_all(&mailbox->ring, data, datasize);

        }

        else
        {

            ring_skip(&mailbox->ring, datasize);

            message->length = 0;

        }

    }

    spinlock_release(&mailbox->spinlock);

    return length ? MESSAGE_OK : MESSAGE_RETRY;

}

unsigned int mailbox_place(struct mailbox *mailbox, struct message *message, void *data)
{

    unsigned int length = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (ring_avail(&mailbox->ring) > message->length)
    {

        length += ring_write_all(&mailbox->ring, message, sizeof (struct message));
        length += ring_write_all(&mailbox->ring, data, message_datasize(message));

    }

    spinlock_release(&mailbox->spinlock);

    return length ? MESSAGE_OK : MESSAGE_RETRY;

}

void mailbox_reset(struct mailbox *mailbox)
{

    spinlock_acquire(&mailbox->spinlock);
    ring_reset(&mailbox->ring);
    spinlock_release(&mailbox->spinlock);

}

void mailbox_register(struct mailbox *mailbox)
{

    resource_register(&mailbox->resource);

}

void mailbox_unregister(struct mailbox *mailbox)
{

    resource_unregister(&mailbox->resource);

}

void mailbox_init(struct mailbox *mailbox, void *buffer, unsigned int count)
{

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox);
    ring_init(&mailbox->ring, count, buffer);
    spinlock_init(&mailbox->spinlock);

}

