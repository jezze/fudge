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

    return MESSAGE_SLOTS - (mailbox->head - mailbox->tail);

}

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

static unsigned int pick(struct mailbox *mailbox, struct message *message)
{

    unsigned int status = MESSAGE_RETRY;
    unsigned int wake = 0;

    spinlock_acquire(&mailbox->spinlock);

    if (mailbox->steps && mailbox->waiters)
    {

        mailbox->waiters = 0;
        wake = 1;

    }

    mailbox->tail += mailbox->steps;
    mailbox->steps = 0;

    if (hasusedslot(mailbox))
    {

        unsigned int slot = mailbox->tail % MESSAGE_SLOTS;

        buffer_copy(message, &mailbox->messages[slot], sizeof (struct message));

        mailbox->steps = 1;

        status = MESSAGE_OK;

    }

    spinlock_release(&mailbox->spinlock);

    if (wake)
        mailbox_wake(mailbox->inode);

    return status;

}

static unsigned int place(struct mailbox *mailbox, unsigned int event, unsigned int source, unsigned int count, void *data)
{

    unsigned int status = MESSAGE_RETRY;

    if (count > MESSAGE_SIZE)
        return MESSAGE_FAILED;

    spinlock_acquire(&mailbox->spinlock);

    if (hasfreeslot(mailbox))
    {

        unsigned int slot = mailbox->head % MESSAGE_SLOTS;
        struct message *message = &mailbox->messages[slot];

        message_init(message, event, source, count, (void *)(unsigned long)(KERNEL_VMAILBOX + MESSAGE_SIZE * slot + MESSAGE_CAPACITY * mailbox->ichannel));
        buffer_copy((void *)(mailbox->data + MESSAGE_SIZE * slot), data, count);

        mailbox->head++;

        status = MESSAGE_OK;

    }

    spinlock_release(&mailbox->spinlock);

    return status;

}

static unsigned int operands_pick(unsigned int source, struct message *message, unsigned int flags)
{

    struct resource *resource = pool_getnoderesource(source);

    if (resource)
    {

        struct mailbox *mailbox = resource->data;

        if (mailbox)
        {

            unsigned int status = pick(mailbox, message);

            if (status == MESSAGE_RETRY && (flags & MESSAGE_FLAG_BLOCK))
                kernel_signal(mailbox->itask, TASK_SIGNAL_BLOCK);

            return status;

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

/* a task whose place found this mailbox full sleeps until a pick frees a slot; fullness is checked and the wait recorded under the mailbox's lock, so a slot freed in between is not missed. Only tasks wait, never the mailbox's own task */
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

            if (!hasfreeslot(mailbox))
            {

                task->waitingfor = target;
                mailbox->waiters++;
                waiting = 1;

            }

            spinlock_release(&mailbox->spinlock);

        }

    }

    if (waiting)
        kernel_signal(itask, TASK_SIGNAL_BLOCK);

    return waiting;

}

/* says whether senders were waiting; the caller wakes them (mailbox_wake) once it holds no task's lock */
unsigned int mailbox_reset(struct mailbox *mailbox)
{

    unsigned int waiters = mailbox->waiters;

    mailbox->waiters = 0;
    mailbox->itask = 0;
    mailbox->ichannel = 0;
    mailbox->head = 0;
    mailbox->tail = 0;
    mailbox->steps = 0;

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

    resource_init(&mailbox->resource, RESOURCE_MAILBOX, mailbox);
    spinlock_init(&mailbox->spinlock);

    mailbox->waiters = 0;

    mailbox_reset(mailbox);

    mailbox->inode = pool_picknode(&mailbox->resource, &operands);
    mailbox->data = data;

}

void mailbox_setup(void)
{

    node_operands_init(&operands, operands_pick, operands_place, 0);

}

