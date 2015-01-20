#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"

static struct list active;
static struct list inactive;
static struct list blocked;

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

