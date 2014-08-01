#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"

static struct list free;
static struct list used;
static struct list blocked;

struct task *scheduler_find_used_task()
{

    if (!used.tail)
        return 0;

    return used.tail->data;

}

struct task *scheduler_find_free_task()
{

    if (!free.tail)
        return 0;

    return free.tail->data;

}

unsigned int scheduler_rendezvous_asleep(struct scheduler_rendezvous *rendezvous)
{

    return rendezvous->task != 0;

}

void scheduler_rendezvous_sleep(struct scheduler_rendezvous *rendezvous, unsigned int condition)
{

    if (rendezvous->task)
        return;

    if (!condition)
        return;

    rendezvous->task = scheduler_find_used_task();

    scheduler_block(rendezvous->task);

}

void scheduler_rendezvous_unsleep(struct scheduler_rendezvous *rendezvous)
{

    if (!rendezvous->task)
        return;

    scheduler_unblock(rendezvous->task);

    rendezvous->task->registers.ip -= 7;
    rendezvous->task = 0;

}

void scheduler_block(struct task *task)
{

    list_move(&blocked, &used, &task->item);

}

void scheduler_unblock(struct task *task)
{

    list_move(&used, &blocked, &task->item);

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

