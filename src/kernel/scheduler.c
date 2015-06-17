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

void scheduler_release(struct task *task)
{

    if (unblock(task))
        task->state.registers.ip -= 7;
    else
        list_move(&active, &active, &task->state.item);

}

unsigned int scheduler_rmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = task_rmessage(task, size, count, buffer);

    if (count)
        scheduler_release(task);
    else
        block(task);

    return count;

}

unsigned int scheduler_wmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = task_wmessage(task, size, count, buffer);

    if (count)
        scheduler_release(task);
    else
        block(task);

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

