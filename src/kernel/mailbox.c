#include <fudge.h>
#include "resource.h"
#include "node.h"
#include "task.h"
#include "pool.h"
#include "mailbox.h"
#include "kernel.h"

static struct node_operands operands;

unsigned int hasusedslot(struct mailbox *mailbox)
{

    return mailbox->head - mailbox->tail;

}

unsigned int hasfreeslot(struct mailbox *mailbox)
{

    return mailbox->slots - (mailbox->head - mailbox->tail);

}

static unsigned int pick(struct mailbox *mailbox, struct message *message, unsigned int count, void *data)
{

    unsigned int status = MESSAGE_RETRY;

    spinlock_acquire(&mailbox->spinlock);

    if (hasusedslot(mailbox))
    {

        unsigned int slot = mailbox->tail % mailbox->slots;
        struct message *m = &mailbox->messages[slot];

        buffer_copy(message, m, sizeof (struct message));

        if (m->length <= count)
        {

            void *mdata = (void *)(((unsigned int)mailbox->data) + 0x1000 * slot);

            buffer_copy(data, mdata, m->length);

            status = MESSAGE_OK;

        }

        else
        {

            status = MESSAGE_TOOBIG;

        }

        mailbox->tail++;

    }

    spinlock_release(&mailbox->spinlock);

    return status;

}

static unsigned int place(struct mailbox *mailbox, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    unsigned int status = MESSAGE_RETRY;

    spinlock_acquire(&mailbox->spinlock);

    if (hasfreeslot(mailbox))
    {

        if (count <= 0x1000)
        {

            unsigned int slot = mailbox->head % mailbox->slots;
            struct message *m = &mailbox->messages[slot];
            void *mdata = (void *)(((unsigned int)mailbox->data) + 0x1000 * slot);

            message_init(m, event, source, count);
            buffer_copy(mdata, data, count);

            mailbox->head++;

            status = MESSAGE_OK;

        }

        else
        {

            status = MESSAGE_TOOBIG;

        }

    }

    spinlock_release(&mailbox->spinlock);

    return status;

}

static unsigned int operands_pick(struct resource *resource, unsigned int source, struct message *message, unsigned int count, void *data)
{

    struct mailbox *mailbox = resource->data;

    if (mailbox)
    {

        unsigned int status = pick(mailbox, message, count, data);

        switch (status)
        {

        case MESSAGE_RETRY:
            kernel_signal(mailbox->itask, TASK_SIGNAL_BLOCK);

            break;

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

        unsigned int status = place(mailbox, event, source, count, data);

        switch (status)
        {

        case MESSAGE_OK:
            kernel_signal(mailbox->itask, TASK_SIGNAL_UNBLOCK);

            break;

        }

        return status;

    }

    return MESSAGE_FAILED;

}

void mailbox_reset(struct mailbox *mailbox, unsigned int itask)
{

    mailbox->itask = itask;
    mailbox->head = 0;
    mailbox->tail = 0;

}

void mailbox_register(struct mailbox *mailbox)
{

    resource_register(&mailbox->resource);

}

void mailbox_unregister(struct mailbox *mailbox)
{

    resource_unregister(&mailbox->resource);

}

void mailbox_init(struct mailbox *mailbox, void *messages, void *data, unsigned int count)
{

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox);
    spinlock_init(&mailbox->spinlock);
    mailbox_reset(mailbox, 0);

    mailbox->messages = messages;
    mailbox->data = data;
    mailbox->slots = (count / 0x1000) - 1;
    mailbox->inode = pool_picknode();

    if (mailbox->inode)
    {

        struct node *node = pool_getnode(mailbox->inode);

        node_reset(node, 0, &mailbox->resource, &operands);

    }

}

void mailbox_setup(void)
{

    node_operands_init(&operands, operands_pick, operands_place);

}

