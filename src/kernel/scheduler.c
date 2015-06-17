#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"

static struct list active;
static struct list inactive;
static struct list blocked;

static unsigned int block(struct task *task)
{

    if (!task->state.blocked)
    {

        list_move(&blocked, &active, &task->state.item);

        task->state.blocked = 1;

        return 1;

    }

    return 0;

}

static unsigned int unblock(struct task *task)
{

    if (task->state.blocked)
    {

        list_move(&active, &blocked, &task->state.item);

        task->state.blocked = 0;

        return 1;

    }

    return 0;

}

static void unblockspecial(struct task *task)
{

    if (unblock(task))
        task->state.registers.ip -= 7;
    else
        list_move(&active, &active, &task->state.item);

}

static unsigned int readbox(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_rcfifo(&task->mailbox.buffer, size, count, buffer);

    if (count)
        unblockspecial(task);
    else
        block(task);

    return count;

}

static unsigned int writebox(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = buffer_wcfifo(&task->mailbox.buffer, size, count, buffer);

    if (count)
        unblockspecial(task);
    else
        block(task);

    return count;

}

static void attach(struct task *task, struct list *mailboxes)
{

    list_add(mailboxes, &task->mailbox.item);

}

static void detach(struct task *task, struct list *mailboxes)
{

    list_remove(mailboxes, &task->mailbox.item);
    unblockspecial(task);

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

unsigned int scheduler_getactiveid()
{

    return (unsigned int)scheduler_findactive();

}

void scheduler_attachactive(struct list *mailboxes)
{

    struct task *task = scheduler_findactive();

    attach(task, mailboxes);

}

void scheduler_detachactive(struct list *mailboxes)
{

    struct task *task = scheduler_findactive();

    detach(task, mailboxes);

}

void scheduler_detachlist(struct list *mailboxes)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        detach(task, mailboxes);

    }

}

unsigned int scheduler_readactive(unsigned int size, unsigned int count, void *buffer)
{

    struct task *task = scheduler_findactive();

    return readbox(task, size, count, buffer);

}

unsigned int scheduler_sendid(unsigned int id, unsigned int size, unsigned int count, void *buffer)
{

    struct task *task = (struct task *)id;

    return writebox(task, size, count, buffer);

}

void scheduler_sendlist(struct list *mailboxes, unsigned int size, unsigned int count, void *buffer)
{

    struct list_item *current;

    for (current = mailboxes->head; current; current = current->next)
    {

        struct task *task = current->data;

        writebox(task, size, count, buffer);

    }

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

