#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"

static struct list active;
static struct list inactive;
static struct list blocked;

static void block(struct task *task)
{

    if (task->blocked)
        return;

    list_move(&blocked, &active, &task->item);

    task->blocked = 1;

}

static void unblock(struct task *task)
{

    if (!task->blocked)
        return;

    list_move(&active, &blocked, &task->item);

    task->blocked = 0;
    task->registers.ip -= 7;

}

struct task *scheduler_findactive()
{

    if (!active.tail)
        return 0;

    return active.tail->data;

}

struct task *scheduler_findinactive()
{

    if (!inactive.tail)
        return 0;

    return inactive.tail->data;

}

unsigned int scheduler_mailboxes_readactive(struct list *mailboxes, unsigned int count, void *buffer)
{

    struct task *task = scheduler_findactive();

    count = buffer_rcfifo(&task->mailbox.buffer, count, buffer);

    if (!count && mailboxes->head != 0)
        block(task);

    return count;

}

void scheduler_mailboxes_addactive(struct list *mailboxes)
{

    struct task *task = scheduler_findactive();

    list_add(mailboxes, &task->mailbox.item);

}

void scheduler_mailboxes_removeactive(struct list *mailboxes)
{

    struct task *task = scheduler_findactive();

    list_remove(mailboxes, &task->mailbox.item);

}

void scheduler_mailboxes_removeall(struct list *mailboxes)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task_mailbox *mailbox = current->data;
        struct task *task = mailbox->owner;

        list_remove(mailboxes, &mailbox->item);
        unblock(task);

    }

}

void scheduler_mailboxes_send(struct list *mailboxes, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = mailboxes->tail; current; current = current->prev)
    {

        struct task_mailbox *mailbox = current->data;
        struct task *task = mailbox->owner;

        buffer_wcfifo(&mailbox->buffer, count, buffer);
        unblock(task);

    }

}

void scheduler_use(struct task *task)
{

    list_move(&active, &inactive, &task->item);

}

void scheduler_unuse(struct task *task)
{

    list_move(&inactive, &active, &task->item);

}

void scheduler_register_task(struct task *task)
{

    list_add(&inactive, &task->item);

}

void scheduler_unregister_task(struct task *task)
{

    list_remove(&inactive, &task->item);

}

void scheduler_setup()
{

    list_init(&active);
    list_init(&inactive);
    list_init(&blocked);

}

