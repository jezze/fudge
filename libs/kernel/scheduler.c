#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"

static struct list active;
static struct list inactive;
static struct list blocked;

unsigned int scheduler_mailbox_write(struct task_mailbox *mailbox, unsigned int count, void *buffer)
{

    struct task *task = mailbox->owner;

    count = buffer_wcfifo(&mailbox->buffer, count, buffer);

    if (count)
        scheduler_unblock(task);

    return count;

}

unsigned int scheduler_mailbox_read(struct task_mailbox *mailbox, unsigned int count, void *buffer)
{

    struct task *task = mailbox->owner;

    count = buffer_rcfifo(&mailbox->buffer, count, buffer);

    if (!count)
        scheduler_block(task);

    return count;

}

void scheduler_activetask_addmailbox(struct list *l)
{

    struct task *task = scheduler_findactivetask();

    list_add(l, &task->mailbox.item);

}

void scheduler_activetask_removemailbox(struct list *l)
{

    struct task *task = scheduler_findactivetask();

    list_remove(l, &task->mailbox.item);

}

unsigned int scheduler_activetask_readmailbox(unsigned int count, void *buffer)
{

    struct task *task = scheduler_findactivetask();

    return scheduler_mailbox_read(&task->mailbox, count, buffer);

}

unsigned int scheduler_activetask_writemailbox(unsigned int count, void *buffer)
{

    struct task *task = scheduler_findactivetask();

    return scheduler_mailbox_write(&task->mailbox, count, buffer);

}

void scheduler_rendezvous_sleep(struct scheduler_rendezvous *rendezvous)
{

    if (rendezvous->task)
        return;

    rendezvous->task = scheduler_findactivetask();

    scheduler_block(rendezvous->task);

}

void scheduler_rendezvous_unsleep(struct scheduler_rendezvous *rendezvous)
{

    if (!rendezvous->task)
        return;

    scheduler_unblock(rendezvous->task);

    rendezvous->task = 0;

}

struct task *scheduler_findactivetask()
{

    if (!active.tail)
        return 0;

    return active.tail->data;

}

struct task *scheduler_findinactivetask()
{

    if (!inactive.tail)
        return 0;

    return inactive.tail->data;

}

void scheduler_block(struct task *task)
{

    if (task->blocked)
        return;

    list_move(&blocked, &active, &task->item);

    task->blocked = 1;

}

void scheduler_unblock(struct task *task)
{

    if (!task->blocked)
        return;

    list_move(&active, &blocked, &task->item);

    task->blocked = 0;
    task->registers.ip -= 7;

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

