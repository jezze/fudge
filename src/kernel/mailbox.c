#include <fudge.h>
#include "resource.h"
#include "task.h"
#include "mmap.h"
#include "pool.h"
#include "service.h"
#include "mailbox.h"
#include "kernel.h"
#include "debug.h"
#include "page.h"

static void reclaim(struct mailbox *mailbox)
{

    unsigned int i;

    spinlock_acquire(&mailbox->spinlock);

    for (i = 0; i < MESSAGE_SLOTS; i++)
    {

        if ((mailbox->held & (1 << i)) && mailbox->done[i])
        {

            mailbox->done[i] = 0;
            mailbox->held &= ~(1 << i);

            list_add_unsafe(&mailbox->free, &mailbox->items[i]);

        }

    }

    spinlock_release(&mailbox->spinlock);

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

        /* a page given to the task has no slot */
        if (message->slot < MESSAGE_SLOTS)
        {

            mailbox->done[message->slot] = 0;
            mailbox->held |= 1 << message->slot;

        }

        n++;

    }

    if (n < count)
        messages[n].type = 0;

    return n;

}

static unsigned int unpromised(struct mailbox *mailbox)
{

    return mailbox->free.count - mailbox->reply - mailbox->reserved;

}

static unsigned int place(struct mailbox *mailbox, unsigned int *promise, unsigned int keep, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    unsigned int status = MESSAGE_FAILED;

    if (mailbox->itask && message_isdata(event))
        return page_deliver(mailbox, event, source, count, data);

    if (count > MESSAGE_CONTROLSIZE)
        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Control message of %u bytes is too long", &count);

    if (mailbox->itask && count <= MESSAGE_CONTROLSIZE)
    {

        status = MESSAGE_RETRY;

        reclaim(mailbox);
        spinlock_acquire(&mailbox->spinlock);

        if (promise && *promise)
            (*promise)--;

        if (unpromised(mailbox) > keep)
        {

            struct list_item *item = mailbox->free.head;
            struct message *message = item->data;

            message_init(message, event, source, count, (void *)(mailbox->vpages + MESSAGE_SLOTSIZE * message->slot));
            buffer_copy((void *)(mailbox->pages + MESSAGE_SLOTSIZE * message->slot), data, count);
            list_move_unsafe(&mailbox->queued, &mailbox->free, item);

            status = MESSAGE_OK;

        }

        spinlock_release(&mailbox->spinlock);

        if (status == MESSAGE_OK)
            kernel_signal(mailbox->itask, TASK_SIGNAL_UNBLOCK);

    }

    return status;

}

static void checkselflock(struct mailbox *mailbox)
{

    struct task *task = pool_gettask(mailbox->itask);

    if (task && !unpromised(mailbox) && !mailbox->reserved && !mailbox->queued.head)
        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "task %u sleeps on its own full mailbox", &mailbox->itask);

}

void mailbox_sleep(struct mailbox *mailbox)
{

    checkselflock(mailbox);
    kernel_signal(mailbox->itask, TASK_SIGNAL_BLOCK);

}

unsigned int mailbox_pick(struct mailbox *mailbox, struct message *messages, unsigned int count, unsigned int flags)
{

    unsigned int n;

    reclaim(mailbox);
    spinlock_acquire(&mailbox->spinlock);

    n = pick(mailbox, messages, count);

    spinlock_release(&mailbox->spinlock);

    if (n)
    {

        unsigned int i;

        for (i = 0; i < n; i++)
        {

            if (messages[i].slot == MESSAGE_SLOTS)
                page_picked(messages[i].data);

        }

        return MESSAGE_OK;

    }

    if (flags & MESSAGE_FLAG_BLOCK)
        mailbox_sleep(mailbox);

    return MESSAGE_RETRY;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct mailbox *mailbox = pool_getnodemailbox(target);

    return (mailbox) ? place(mailbox, 0, MAILBOX_REPLYSLOTS, event, source, count, data) : MESSAGE_FAILED;

}

static unsigned int shift(struct mailbox *mailbox, unsigned int *from, unsigned int *to)
{

    unsigned int moved = 0;

    reclaim(mailbox);
    spinlock_acquire(&mailbox->spinlock);

    if ((from) ? *from : unpromised(mailbox))
    {

        if (from)
            (*from)--;

        if (to)
            (*to)++;

        moved = 1;

    }

    spinlock_release(&mailbox->spinlock);

    return moved;

}

unsigned int mailbox_reservereply(struct mailbox *mailbox)
{

    if (mailbox->itask)
        return (mailbox->reply || shift(mailbox, 0, &mailbox->reply)) ? MESSAGE_OK : MESSAGE_RETRY;

    return MESSAGE_FAILED;

}

void mailbox_unreservereply(struct mailbox *mailbox)
{

    shift(mailbox, &mailbox->reply, 0);

}

unsigned int mailbox_keepreply(struct mailbox *mailbox)
{

    return shift(mailbox, &mailbox->reply, &mailbox->reserved) ? MESSAGE_OK : MESSAGE_FAILED;

}

void mailbox_unreserve(struct mailbox *mailbox)
{

    shift(mailbox, &mailbox->reserved, 0);

}

unsigned int mailbox_placenotify(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    /* notifies also leave a slot to task mail: a slot freed in a flooded mailbox goes to a task retrying a send, not to the next event */
    return place(mailbox, 0, MAILBOX_REPLYSLOTS + MAILBOX_MAILSLOTS, event, source, count, data);

}

unsigned int mailbox_placereply(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    return place(mailbox, &mailbox->reply, 0, event, source, count, data);

}

unsigned int mailbox_placereserved(struct mailbox *mailbox, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    return place(mailbox, &mailbox->reserved, 0, event, source, count, data);

}

void mailbox_reset(struct mailbox *mailbox)
{

    unsigned int i;

    mailbox->itask = 0;
    mailbox->ichannel = 0;
    mailbox->held = 0;
    mailbox->reply = 0;
    mailbox->reserved = 0;

    list_init(&mailbox->free);
    list_init(&mailbox->queued);

    for (i = 0; i < MESSAGE_SLOTS; i++)
        list_add_unsafe(&mailbox->free, &mailbox->items[i]);

}

void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel, unsigned long pages)
{

    unsigned int i;

    mailbox->itask = itask;
    mailbox->ichannel = ichannel;
    mailbox->pages = pages + MESSAGE_SLOTSIZE * MESSAGE_SLOTS * ichannel;
    mailbox->vpages = KERNEL_VMAILBOX + MESSAGE_SLOTSIZE * MESSAGE_SLOTS * ichannel;
    mailbox->done = (volatile unsigned char *)(pages + MESSAGE_SLOTSIZE * MESSAGE_PAGES + MESSAGE_SLOTS * ichannel);

    for (i = 0; i < MESSAGE_SLOTS; i++)
        mailbox->done[i] = 0;

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

