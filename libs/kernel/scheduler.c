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

    if (!task->state.blocked)
    {

        list_move(&blocked, &active, &task->state.item);

        task->state.blocked = 1;

    }

}

static void unblock(struct task *task)
{

    if (task->state.blocked)
    {

        list_move(&active, &blocked, &task->state.item);

        task->state.registers.ip -= 7;
        task->state.blocked = 0;

    }

    list_move(&active, &active, &task->state.item);

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

void scheduler_attach(struct list *mailboxes)
{

    struct task *task = active.tail->data;

    list_add(mailboxes, &task->mailbox.item);

}

void scheduler_detach(struct list *mailboxes)
{

    struct task *task = active.tail->data;

    list_remove(mailboxes, &task->mailbox.item);

}

void scheduler_detachall(struct list *mailboxes)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        list_remove(mailboxes, &task->mailbox.item);
        unblock(task);

    }

}

unsigned int scheduler_getactiveid()
{

    return (unsigned int)active.tail->data;

}

unsigned int scheduler_readactive(unsigned int count, void *buffer)
{

    struct task *task = active.tail->data;

    count = buffer_rcfifo(&task->mailbox.buffer, count, buffer);

    if (!count)
        block(task);

    return count;

}

unsigned int scheduler_sendid(unsigned int id, unsigned int count, void *buffer)
{

    struct task *task = (struct task *)id;

    unblock(task);

    return buffer_wcfifo(&task->mailbox.buffer, count, buffer);

}

void scheduler_sendlist(struct list *mailboxes, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        buffer_wcfifo(&task->mailbox.buffer, count, buffer);

        unblock(task);

    }

}

unsigned int scheduler_readpipe(struct list *mailboxes, unsigned int count, void *buffer)
{

    struct task *task = active.tail->data;

    count = buffer_rcfifo(&task->mailbox.buffer, count, buffer);

    if (!count && mailboxes->head != 0)
        block(task);

    return count;

}

unsigned int scheduler_sendpipe(struct list *mailboxes, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        count = buffer_wcfifo(&task->mailbox.buffer, count, buffer);

        unblock(task);

    }

    return count;

}

void scheduler_use(struct task *task)
{

    list_move(&active, &inactive, &task->state.item);

}

void scheduler_unuse(struct task *task)
{

    list_move(&inactive, &active, &task->state.item);

}

void scheduler_registertask(struct task *task)
{

    list_add(&inactive, &task->state.item);

}

void scheduler_unregistertask(struct task *task)
{

    list_remove(&inactive, &task->state.item);

}

void scheduler_setup()
{

    list_init(&active);
    list_init(&inactive);
    list_init(&blocked);

}

