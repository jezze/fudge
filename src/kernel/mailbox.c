#include <fudge.h>
#include "resource.h"
#include "mailbox.h"

unsigned int mailbox_pick(struct mailbox *mailbox, struct message_header *header, void *data)
{

    unsigned int count = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (ring_count(&mailbox->ring))
    {

        count += ring_readall(&mailbox->ring, header, sizeof (struct message_header));
        count += ring_readall(&mailbox->ring, data, message_datasize(header));

    }

    spinlock_release(&mailbox->spinlock);

    return count;

}

unsigned int mailbox_place(struct mailbox *mailbox, struct message_header *header, void *data)
{

    unsigned int count = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (ring_avail(&mailbox->ring) > header->length)
    {

        count += ring_writeall(&mailbox->ring, header, sizeof (struct message_header));
        count += ring_writeall(&mailbox->ring, data, message_datasize(header));

    }

    spinlock_release(&mailbox->spinlock);

    return count;

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

void mailbox_init(struct mailbox *mailbox, char *buffer, unsigned int count)
{

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox);
    ring_init(&mailbox->ring, count, buffer);
    spinlock_init(&mailbox->spinlock);
    list_inititem(&mailbox->item, mailbox);

}

