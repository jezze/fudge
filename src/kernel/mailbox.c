#include <fudge.h>
#include "mailbox.h"

unsigned int mailbox_pick(struct mailbox *mailbox, struct event_header *header, void *data)
{

    unsigned int count = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (ring_count(&mailbox->ring))
    {

        count += ring_readall(&mailbox->ring, header, sizeof (struct event_header));
        count += ring_readall(&mailbox->ring, data, header->length - sizeof (struct event_header));

    }

    spinlock_release(&mailbox->spinlock);

    return count;

}

unsigned int mailbox_place(struct mailbox *mailbox, struct event_header *header, void *data)
{

    unsigned int count = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (ring_avail(&mailbox->ring) > header->length)
    {

        count += ring_writeall(&mailbox->ring, header, sizeof (struct event_header));
        count += ring_writeall(&mailbox->ring, data, header->length - sizeof (struct event_header));

    }

    spinlock_release(&mailbox->spinlock);

    return count;

}

void mailbox_init(struct mailbox *mailbox, unsigned int count, char *buffer)
{

    ring_init(&mailbox->ring, count, buffer);

}

