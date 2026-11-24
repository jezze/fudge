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

/* mail is copied out with its payload and its slot is free again at once; a page given to the task stays where it is */
static unsigned int pick(struct mailbox *mailbox, struct message_box *boxes, unsigned int count)
{

    unsigned int n = 0;

    while (n < count && mailbox->queued.head)
    {

        struct list_item *item = mailbox->queued.head;
        struct message *message = item->data;

        list_remove_unsafe(&mailbox->queued, item);
        buffer_copy(&boxes[n].message, message, sizeof (struct message));

        if (!message->page)
        {

            struct message_box *box = item->data;

            buffer_copy(boxes[n].payload, box->payload, message->length);

            boxes[n].message.data = boxes[n].payload;

            list_add_unsafe(&mailbox->free, item);

        }

        n++;

    }

    if (n < count)
        boxes[n].message.type = 0;

    return n;

}

static unsigned int unpromised(struct mailbox *mailbox)
{

    return mailbox->free.count - mailbox->reply - mailbox->reserved;

}

/* slots a placement leaves free: task mail leaves the reply slot; notifies also leave one to task mail, so a slot freed in a flooded mailbox goes to a task retrying a send, not to the next event */
static unsigned int getkeep(unsigned int mode)
{

    switch (mode)
    {

    case MAILBOX_NOTIFY:
        return MAILBOX_REPLYSLOTS + MAILBOX_MAILSLOTS;

    case MAILBOX_MAIL:
        return MAILBOX_REPLYSLOTS;

    }

    return 0;

}

unsigned int mailbox_place(struct mailbox *mailbox, unsigned int mode, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    unsigned int status = MESSAGE_FAILED;

    if (mailbox->itask && message_isdata(event))
        return page_deliver(mailbox, event, source, count, data);

    if (count > MESSAGE_CONTROLSIZE)
        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Control message of %u bytes is too long", &count);

    if (mailbox->itask && count <= MESSAGE_CONTROLSIZE)
    {

        status = MESSAGE_RETRY;

        spinlock_acquire(&mailbox->spinlock);

        if (mode == MAILBOX_REPLY)
            mailbox->reply = 0;

        if (mode == MAILBOX_KEPT && mailbox->reserved)
            mailbox->reserved--;

        if (unpromised(mailbox) > getkeep(mode))
        {

            struct list_item *item = mailbox->free.head;
            struct message_box *box = item->data;

            message_init(&box->message, event, source, count, 0);
            buffer_copy(box->payload, data, count);
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

unsigned int mailbox_pick(struct mailbox *mailbox, struct message_box *boxes, unsigned int count, unsigned int flags)
{

    unsigned int n;

    spinlock_acquire(&mailbox->spinlock);

    n = pick(mailbox, boxes, count);

    spinlock_release(&mailbox->spinlock);

    if (n)
    {

        unsigned int i;

        for (i = 0; i < n; i++)
        {

            if (boxes[i].message.page)
                page_picked(boxes[i].message.page);

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

    return (mailbox) ? mailbox_place(mailbox, MAILBOX_MAIL, source, event, count, data) : MESSAGE_FAILED;

}

/* the one reply slot of a call into the kernel is promised before the call is run */
unsigned int mailbox_reservereply(struct mailbox *mailbox)
{

    unsigned int status;

    if (!mailbox->itask)
        return MESSAGE_FAILED;

    spinlock_acquire(&mailbox->spinlock);

    if (!mailbox->reply && unpromised(mailbox))
        mailbox->reply = 1;

    status = (mailbox->reply) ? MESSAGE_OK : MESSAGE_RETRY;

    spinlock_release(&mailbox->spinlock);

    return status;

}

void mailbox_unreservereply(struct mailbox *mailbox)
{

    spinlock_acquire(&mailbox->spinlock);

    mailbox->reply = 0;

    spinlock_release(&mailbox->spinlock);

}

/* a service that answers later keeps the promised slot past the call */
unsigned int mailbox_keepreply(struct mailbox *mailbox)
{

    unsigned int status;

    spinlock_acquire(&mailbox->spinlock);

    status = (mailbox->reply) ? MESSAGE_OK : MESSAGE_FAILED;

    if (mailbox->reply)
    {

        mailbox->reply = 0;
        mailbox->reserved++;

    }

    spinlock_release(&mailbox->spinlock);

    return status;

}

void mailbox_unreserve(struct mailbox *mailbox)
{

    spinlock_acquire(&mailbox->spinlock);

    if (mailbox->reserved)
        mailbox->reserved--;

    spinlock_release(&mailbox->spinlock);

}

void mailbox_reset(struct mailbox *mailbox)
{

    unsigned int i;

    mailbox->itask = 0;
    mailbox->ichannel = 0;
    mailbox->reply = 0;
    mailbox->reserved = 0;

    list_init(&mailbox->free);
    list_init(&mailbox->queued);

    for (i = 0; i < MESSAGE_SLOTS; i++)
        list_add_unsafe(&mailbox->free, &mailbox->items[i]);

}

void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel)
{

    mailbox->itask = itask;
    mailbox->ichannel = ichannel;

}

void mailbox_init(struct mailbox *mailbox)
{

    unsigned int i;

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox, operands_place);
    spinlock_init(&mailbox->spinlock);

    for (i = 0; i < MESSAGE_SLOTS; i++)
    {

        list_inititem(&mailbox->items[i], &mailbox->boxes[i]);

        mailbox->boxes[i].message.page = 0;

    }

    mailbox_reset(mailbox);

    mailbox->resource.inode = pool_picknode(&mailbox->resource);

}

