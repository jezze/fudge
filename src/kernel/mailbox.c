#include <fudge.h>
#include "resource.h"
#include "node.h"
#include "task.h"
#include "pool.h"
#include "mailbox.h"

static struct node_operands operands;

static unsigned int operands_pick(struct resource *resource, unsigned int source, struct message *message, unsigned int count, void *data)
{

    struct mailbox *mailbox = resource->data;

    if (mailbox)
    {

        unsigned int status = mailbox_pick(mailbox, message, count, data);

        if (status == MESSAGE_RETRY)
        {

            struct task *task = pool_gettask(mailbox->itask);

            if (task)
                task_signal(task, TASK_SIGNAL_BLOCK);

        }

        return status;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(struct resource *resource, unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct mailbox *mailbox = resource->data;

    if (mailbox)
    {

        unsigned int status = mailbox_place(mailbox, event, source, count, data);
        struct task *task = pool_gettask(mailbox->itask);

        if (task)
            task_signal(task, TASK_SIGNAL_UNBLOCK);

        return status;

    }

    return MESSAGE_FAILED;

}

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

            length = 0;

        }

    }

    spinlock_release(&mailbox->spinlock);

    return length ? MESSAGE_OK : MESSAGE_RETRY;

}

unsigned int mailbox_place(struct mailbox *mailbox, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    unsigned int length = 0;
    struct message message;

    message_init(&message, event, source, count);
    spinlock_acquire(&mailbox->spinlock);

    if (ring_avail(&mailbox->ring) > message.length)
    {

        length += ring_write_all(&mailbox->ring, &message, sizeof (struct message));
        length += ring_write_all(&mailbox->ring, data, message_datasize(&message));

    }

    spinlock_release(&mailbox->spinlock);

    return length ? MESSAGE_OK : MESSAGE_RETRY;

}

void mailbox_reset(struct mailbox *mailbox, unsigned int itask)
{

    spinlock_acquire(&mailbox->spinlock);
    ring_reset(&mailbox->ring);
    spinlock_release(&mailbox->spinlock);

    mailbox->itask = itask;

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
    mailbox_reset(mailbox, 0);

    mailbox->inode = pool_addnode("mailbox", &mailbox->resource, &operands);

}

void mailbox_setup(void)
{

    node_operands_init(&operands, operands_pick, operands_place);

}

