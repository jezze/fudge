#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"

static struct list free;
static struct list used;
static struct list blocked;

void scheduler_rendezvous_sleep(struct scheduler_rendezvous *rendezvous)
{

    if (rendezvous->task)
        return;

    rendezvous->task = scheduler_findusedtask();

    scheduler_block(rendezvous->task);

}

void scheduler_rendezvous_unsleep(struct scheduler_rendezvous *rendezvous)
{

    if (!rendezvous->task)
        return;

    scheduler_unblock(rendezvous->task);

    rendezvous->task = 0;

}

struct task *scheduler_findusedtask()
{

    if (!used.tail)
        return 0;

    return used.tail->data;

}

struct task *scheduler_findfreetask()
{

    if (!free.tail)
        return 0;

    return free.tail->data;

}

void scheduler_block(struct task *task)
{

    list_move(&blocked, &used, &task->item);

}

void scheduler_unblock(struct task *task)
{

    list_move(&used, &blocked, &task->item);

    task->registers.ip -= 7;

}

void scheduler_use(struct task *task)
{

    list_move(&used, &free, &task->item);

}

void scheduler_unuse(struct task *task)
{

    list_move(&free, &used, &task->item);

}

void scheduler_register_task(struct task *task)
{

    list_add(&free, &task->item);

}

void scheduler_unregister_task(struct task *task)
{

    list_remove(&free, &task->item);

}

void scheduler_setup()
{

    list_init(&free);
    list_init(&used);
    list_init(&blocked);

}

