#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "mmap.h"
#include "pool.h"
#include "service.h"
#include "mailbox.h"
#include "kernel.h"
#include "debug.h"

static unsigned int release(struct mailbox *mailbox, unsigned int slots)
{

    unsigned int i;

    if (slots & ~mailbox->held)
        return MESSAGE_FAILED;

    for (i = 0; i < MESSAGE_SLOTS; i++)
    {

        if (slots & (1 << i))
            list_add_unsafe(&mailbox->free, &mailbox->items[i]);

    }

    mailbox->held &= ~slots;

    return MESSAGE_OK;

}

static unsigned int pick(struct mailbox *mailbox, struct message *messages, unsigned int count)
{

    unsigned int n = 0;

    while (n < count && mailbox->queued.head)
    {

        struct list_item *item = mailbox->queued.head;
        struct message *message = item->data;

        buffer_copy(&messages[n], message, sizeof (struct message));
        list_remove_unsafe(&mailbox->queued, item);

        mailbox->held |= 1 << message->slot;

        n++;

    }

    if (n < count)
        messages[n].type = 0;

    return n;

}

static unsigned int place(struct mailbox *mailbox, struct list *from, unsigned int keep, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    if (count <= MESSAGE_SIZE)
    {

        unsigned int status = MESSAGE_RETRY;

        spinlock_acquire(&mailbox->spinlock);

        if (from->count > keep)
        {

            struct list_item *item = from->head;
            struct message *message = item->data;

            message_init(message, event, source, count, (void *)(unsigned long)(KERNEL_VMAILBOX + MESSAGE_SIZE * (MESSAGE_SLOTS * mailbox->ichannel + message->slot)));
            buffer_copy((void *)(mailbox->pages + MESSAGE_SIZE * message->slot), data, count);
            list_move_unsafe(&mailbox->queued, from, item);

            status = MESSAGE_OK;

        }

        spinlock_release(&mailbox->spinlock);

        return status;

    }

    return MESSAGE_FAILED;

}

static void checkselflock(struct mailbox *mailbox)
{

    struct task *task = pool_gettask(mailbox->itask);

    /* only the task itself could free a slot here, and it is about to sleep */
    if (task && !task->waitingfor && !mailbox->free.count && !mailbox->queued.head && !mailbox->reserved.head)
        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "task %u sleeps on its own full mailbox", &mailbox->itask);

}

static void checkcycle(struct mailbox *mailbox, unsigned int itask)
{

    unsigned int owner = mailbox->itask;
    unsigned int i;

    for (i = 0; i < POOL_TASKS; i++)
    {

        struct task *task = pool_gettask(owner);
        struct mailbox *next;

        if (owner == itask)
        {

            debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "task %u waits on task %u, which waits on it", &itask, &mailbox->itask);

            return;

        }

        if (!task || !task->waitingfor)
            return;

        next = pool_getnodemailbox(task->waitingfor);

        if (!next)
            return;

        owner = next->itask;

    }

}

unsigned int mailbox_pick(struct mailbox *mailbox, struct message *messages, unsigned int count, unsigned int slots, unsigned int flags)
{

    unsigned int status;
    unsigned int n = 0;

    spinlock_acquire(&mailbox->spinlock);

    status = release(mailbox, slots);

    if (status == MESSAGE_OK)
        n = pick(mailbox, messages, count);

    spinlock_release(&mailbox->spinlock);

    if (status != MESSAGE_OK)
        return status;

    if (slots)
        kernel_wake(mailbox->resource.inode);

    if (n)
        return MESSAGE_OK;

    if ((flags & MESSAGE_FLAG_BLOCK) && !count)
        checkselflock(mailbox);

    if (flags & MESSAGE_FLAG_BLOCK)
        kernel_signal(mailbox->itask, TASK_SIGNAL_BLOCK);

    return MESSAGE_RETRY;

}

static unsigned int deliver(struct mailbox *mailbox, struct list *from, unsigned int keep, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    if (mailbox->itask)
    {

        unsigned int status = place(mailbox, from, keep, event, source, count, data);

        if (status == MESSAGE_OK)
            kernel_signal(mailbox->itask, TASK_SIGNAL_UNBLOCK);

        return status;

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct mailbox *mailbox = pool_getnodemailbox(target);

    /* task mail and notifies leave the last free slot to a reply, so a task flooded with them can still reserve one (mailbox_reservereply) and is never left waiting on its own mailbox */
    return (mailbox) ? deliver(mailbox, &mailbox->free, MAILBOX_REPLYSLOTS, event, source, count, data) : MESSAGE_FAILED;

}

static unsigned int move(struct mailbox *mailbox, struct list *to, struct list *from)
{

    unsigned int moved = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (from->head)
    {

        list_move_unsafe(to, from, from->head);

        moved = 1;

    }

    spinlock_release(&mailbox->spinlock);

    return moved;

}

unsigned int mailbox_reservereply(struct mailbox *mailbox)
{

    if (mailbox->itask)
        return (mailbox->reply.head || move(mailbox, &mailbox->reply, &mailbox->free)) ? MESSAGE_OK : MESSAGE_RETRY;

    return MESSAGE_FAILED;

}

void mailbox_unreservereply(struct mailbox *mailbox)
{

    if (move(mailbox, &mailbox->free, &mailbox->reply))
        kernel_wake(mailbox->resource.inode);

}

unsigned int mailbox_keepreply(struct mailbox *mailbox)
{

    return move(mailbox, &mailbox->reserved, &mailbox->reply) ? MESSAGE_OK : MESSAGE_FAILED;

}

void mailbox_unreserve(struct mailbox *mailbox)
{

    if (move(mailbox, &mailbox->free, &mailbox->reserved))
        kernel_wake(mailbox->resource.inode);

}

unsigned int mailbox_placereply(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    return deliver(mailbox, (mailbox->reply.head) ? &mailbox->reply : &mailbox->free, 0, event, source, count, data);

}

unsigned int mailbox_placereserved(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    return deliver(mailbox, &mailbox->reserved, 0, event, source, count, data);

}

unsigned int mailbox_wait(struct mailbox *mailbox, unsigned int itask)
{

    struct task *task = pool_gettask(itask);
    unsigned int waiting = 0;

    if (task && mailbox->itask && mailbox->itask != itask)
    {

        spinlock_acquire(&mailbox->spinlock);

        if (mailbox->free.count <= MAILBOX_REPLYSLOTS)
        {

            task->waitingfor = mailbox->resource.inode;
            waiting = 1;

        }

        spinlock_release(&mailbox->spinlock);

        if (waiting)
            checkcycle(mailbox, itask);

    }

    return waiting;

}

void mailbox_reset(struct mailbox *mailbox)
{

    unsigned int i;

    mailbox->itask = 0;
    mailbox->ichannel = 0;
    mailbox->held = 0;

    list_init(&mailbox->free);
    list_init(&mailbox->queued);
    list_init(&mailbox->reply);
    list_init(&mailbox->reserved);

    for (i = 0; i < MESSAGE_SLOTS; i++)
        list_add_unsafe(&mailbox->free, &mailbox->items[i]);

}

void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel, unsigned long pages)
{

    mailbox->itask = itask;
    mailbox->ichannel = ichannel;
    mailbox->pages = pages + MESSAGE_SIZE * MESSAGE_SLOTS * ichannel;

}

void mailbox_init(struct mailbox *mailbox)
{

    unsigned int i;

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox, operands_place);
    spinlock_init(&mailbox->spinlock);

    for (i = 0; i < MESSAGE_SLOTS; i++)
    {

        list_inititem(&mailbox->items[i], &mailbox->messages[i]);

        mailbox->messages[i].slot = i;

    }

    mailbox_reset(mailbox);

    mailbox->resource.inode = pool_picknode(&mailbox->resource);

}

