#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"
#include "scheduler.h"

static struct list active;
static struct list inactive;
static struct list blocked;

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

unsigned int scheduler_block(struct task *task)
{

    if (!task->state.blocked)
    {

        list_move(&blocked, &active, &task->state.item);

        task->state.blocked = 1;

        return 1;

    }

    return 0;

}

unsigned int scheduler_unblock(struct task *task)
{

    if (task->state.blocked)
    {

        list_move(&active, &blocked, &task->state.item);

        task->state.blocked = 0;

        return 1;

    }

    return 0;

}

void scheduler_unblockspecial(struct task *task)
{

    if (scheduler_unblock(task))
        task->state.registers.ip -= 7;
    else
        list_move(&active, &active, &task->state.item);

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

