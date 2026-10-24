#include <fudge.h>
#include "resource.h"
#include "node.h"
#include "task.h"
#include "pool.h"
#include "mailbox.h"
#include "kernel.h"

static struct node_operands operands;

/* senders that found the mailbox full go back to running and try again; callers hold no task's lock, since this takes the waiters' */
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

/* slots move from free to queued (place), to picked (pick) and back to free when the task releases them, in any order; only picked slots can be released */
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

    unsigned int wake = 0;
    unsigned int n = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (release(mailbox, slots) && mailbox->waiters)
    {

        mailbox->waiters = 0;
        wake = 1;

    }

    while (n < count && mailbox->queued.head)
    {

        struct list_item *item = mailbox->queued.head;

        buffer_copy(&messages[n], item->data, sizeof (struct message));
        list_move_unsafe(&mailbox->picked, &mailbox->queued, item);

        n++;

    }

    if (n < count)
        messages[n].event = 0;

    spinlock_release(&mailbox->spinlock);

    if (wake)
        mailbox_wake(mailbox->inode);

    return n;

}

static unsigned int place(struct mailbox *mailbox, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    unsigned int status = MESSAGE_RETRY;

    if (count > MESSAGE_SIZE)
        return MESSAGE_FAILED;

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

static unsigned int operands_pick(unsigned int source, struct message *messages, unsigned int count, unsigned int slots, unsigned int flags)
{

    struct resource *resource = pool_getnoderesource(source);

    if (resource)
    {

        struct mailbox *mailbox = resource->data;

        if (mailbox)
        {

            if (pick(mailbox, messages, count, slots))
                return MESSAGE_OK;

            if (flags & MESSAGE_FLAG_BLOCK)
                kernel_signal(mailbox->itask, TASK_SIGNAL_BLOCK);

            return MESSAGE_RETRY;

        }

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct mailbox *mailbox = resource->data;

        if (mailbox && mailbox->itask)
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

    }

    return MESSAGE_FAILED;

}

/* records a task whose place found this mailbox full, so a pick that frees a slot wakes it; fullness is checked and the wait recorded under the mailbox's lock, so a slot freed in between is not missed. The task itself sleeps in pick, where mail for it wakes it too. Only tasks wait, never the mailbox's own task */
unsigned int mailbox_wait(unsigned int target, unsigned int itask)
{

    struct resource *resource = pool_getnoderesource(target);
    struct task *task = pool_gettask(itask);
    unsigned int waiting = 0;

    if (resource && resource->type == RESOURCE_MAILBOX && task)
    {

        struct mailbox *mailbox = resource->data;

        if (mailbox && mailbox->itask && mailbox->itask != itask)
        {

            spinlock_acquire(&mailbox->spinlock);

            if (!mailbox->free.head)
            {

                task->waitingfor = target;
                mailbox->waiters++;
                waiting = 1;

            }

            spinlock_release(&mailbox->spinlock);

        }

    }

    return waiting;

}

/* says whether senders were waiting; the caller wakes them (mailbox_wake) once it holds no task's lock */
unsigned int mailbox_reset(struct mailbox *mailbox)
{

    unsigned int waiters = mailbox->waiters;
    unsigned int i;

    mailbox->waiters = 0;
    mailbox->itask = 0;
    mailbox->ichannel = 0;

    list_init(&mailbox->free);
    list_init(&mailbox->queued);
    list_init(&mailbox->picked);

    for (i = 0; i < MESSAGE_SLOTS; i++)
        list_add_unsafe(&mailbox->free, &mailbox->items[i]);

    return waiters;

}

void mailbox_register(struct mailbox *mailbox, unsigned int itask, unsigned int ichannel)
{

    resource_register(&mailbox->resource);

    mailbox->itask = itask;
    mailbox->ichannel = ichannel;

}

void mailbox_unregister(struct mailbox *mailbox)
{

    resource_unregister(&mailbox->resource);

    mailbox->itask = 0;
    mailbox->ichannel = 0;

}

void mailbox_init(struct mailbox *mailbox, unsigned long data)
{

    unsigned int i;

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox);
    spinlock_init(&mailbox->spinlock);

    for (i = 0; i < MESSAGE_SLOTS; i++)
    {

        mailbox->messages[i].slot = i;

        list_inititem(&mailbox->items[i], &mailbox->messages[i]);

    }

    mailbox_reset(mailbox);

    mailbox->inode = pool_picknode(&mailbox->resource, &operands);
    mailbox->data = data;

}

void mailbox_setup(void)
{

    node_operands_init(&operands, operands_pick, operands_place, 0);

}

