#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "mmap.h"
#include "pool.h"
#include "service.h"
#include "mailbox.h"
#include "kernel.h"

static struct resource_operands operands;

struct mailbox *mailbox_get(unsigned int inode)
{

    struct resource *resource = pool_getnoderesource(inode);

    return (resource && resource->type == RESOURCE_MAILBOX) ? resource->data : 0;

}

void mailbox_wake(unsigned int inode)
{

    unsigned int i;

    for (i = 1; i < POOL_TASKS; i++)
    {

        struct task *task = pool_gettask(i);

        if (task && task->waitingfor == inode)
        {

            task->waitingfor = 0;

            kernel_signal(i, TASK_SIGNAL_UNBLOCK);

        }

    }

}

static unsigned int release(struct mailbox *mailbox, unsigned int slots)
{

    struct list_item *current = mailbox->picked.head;
    unsigned int count = 0;

    while (current)
    {

        struct list_item *next = current->next;
        struct message *message = current->data;

        if (slots & (1 << message->slot))
        {

            list_move_unsafe(&mailbox->free, &mailbox->picked, current);

            count++;

        }

        current = next;

    }

    return count;

}

static unsigned int pick(struct mailbox *mailbox, struct message *messages, unsigned int count, unsigned int slots)
{

    unsigned int released;
    unsigned int n = 0;

    spinlock_acquire(&mailbox->spinlock);

    released = release(mailbox, slots);

    while (n < count && mailbox->queued.head)
    {

        struct list_item *item = mailbox->queued.head;

        buffer_copy(&messages[n], item->data, sizeof (struct message));
        list_move_unsafe(&mailbox->picked, &mailbox->queued, item);

        n++;

    }

    if (n < count)
        messages[n].type = 0;

    spinlock_release(&mailbox->spinlock);

    if (released)
        mailbox_wake(mailbox->resource.inode);

    return n;

}

static unsigned int place(struct mailbox *mailbox, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    if (count <= MESSAGE_SIZE)
    {

        unsigned int status = MESSAGE_RETRY;

        spinlock_acquire(&mailbox->spinlock);

        if (mailbox->free.head)
        {

            struct list_item *item = mailbox->free.head;
            struct message *message = item->data;

            message_init(message, event, source, count, (void *)(unsigned long)(KERNEL_VMAILBOX + MESSAGE_SIZE * message->slot + MESSAGE_CAPACITY * mailbox->ichannel));
            buffer_copy((void *)(mailbox->data + MESSAGE_SIZE * message->slot), data, count);
            list_move_unsafe(&mailbox->queued, &mailbox->free, item);

            status = MESSAGE_OK;

        }

        spinlock_release(&mailbox->spinlock);

        return status;

    }

    return MESSAGE_FAILED;

}

unsigned int mailbox_pick(unsigned int source, struct message *messages, unsigned int count, unsigned int slots, unsigned int flags)
{

    struct mailbox *mailbox = mailbox_get(source);

    if (mailbox)
    {

        if (pick(mailbox, messages, count, slots))
            return MESSAGE_OK;

        if (flags & MESSAGE_FLAG_BLOCK)
            kernel_signal(mailbox->itask, TASK_SIGNAL_BLOCK);

        return MESSAGE_RETRY;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct mailbox *mailbox = mailbox_get(target);

    if (mailbox)
    {

        if (mailbox->itask)
        {

            unsigned int status = place(mailbox, event, source, count, data);

            if (status == MESSAGE_OK)
                kernel_signal(mailbox->itask, TASK_SIGNAL_UNBLOCK);

            return status;

        }

    }

    return MESSAGE_FAILED;

}

unsigned int mailbox_isfull(unsigned int inode)
{

    struct mailbox *mailbox = mailbox_get(inode);
    unsigned int full = 0;

    if (mailbox)
    {

        spinlock_acquire(&mailbox->spinlock);

        full = !mailbox->free.head;

        spinlock_release(&mailbox->spinlock);

    }

    return full;

}

unsigned int mailbox_wait(unsigned int target, unsigned int itask)
{

    struct mailbox *mailbox = mailbox_get(target);
    struct task *task = pool_gettask(itask);
    unsigned int waiting = 0;

    if (mailbox && task)
    {

        if (mailbox->itask && mailbox->itask != itask)
        {

            spinlock_acquire(&mailbox->spinlock);

            if (!mailbox->free.head)
            {

                task->waitingfor = target;
                waiting = 1;

            }

            spinlock_release(&mailbox->spinlock);

        }

    }

    return waiting;

}

void mailbox_reset(struct mailbox *mailbox)
{

    unsigned int i;

    mailbox->itask = 0;
    mailbox->ichannel = 0;

    list_init(&mailbox->free);
    list_init(&mailbox->queued);
    list_init(&mailbox->picked);

    for (i = 0; i < MESSAGE_SLOTS; i++)
        list_add_unsafe(&mailbox->free, &mailbox->items[i]);

}

void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel)
{

    mailbox->itask = itask;
    mailbox->ichannel = ichannel;

}

void mailbox_init(struct mailbox *mailbox, unsigned long data)
{

    unsigned int i;

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox, &operands);
    spinlock_init(&mailbox->spinlock);

    for (i = 0; i < MESSAGE_SLOTS; i++)
    {

        mailbox->messages[i].slot = i;

        list_inititem(&mailbox->items[i], &mailbox->messages[i]);

    }

    mailbox_reset(mailbox);

    mailbox->resource.inode = pool_picknode(&mailbox->resource);
    mailbox->data = data;

}

void mailbox_setup(void)
{

    resource_operands_init(&operands, operands_place, 0);

}

