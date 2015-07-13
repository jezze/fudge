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

void scheduler_setstatus(struct task *task, unsigned int status)
{

    switch (status)
    {

    case TASK_STATUS_BLOCKED:
        switch (task->state.status)
        {

        case TASK_STATUS_READY:
        case TASK_STATUS_ACTIVE:
            list_move(&blocked, &active, &task->state.item);

            task->state.status = TASK_STATUS_BLOCKED;

            break;

        }

        break;

    case TASK_STATUS_ACTIVE:
        switch (task->state.status)
        {

        case TASK_STATUS_INACTIVE:
            list_move(&active, &inactive, &task->state.item);

            task->state.status = TASK_STATUS_ACTIVE;

            break;

        case TASK_STATUS_BLOCKED:
            list_move(&active, &blocked, &task->state.item);

            task->state.status = TASK_STATUS_READY;

            break;

        case TASK_STATUS_READY:
        case TASK_STATUS_ACTIVE:
            list_move(&active, &active, &task->state.item);

            break;

        }

        break;

    case TASK_STATUS_INACTIVE:
        list_move(&inactive, &active, &task->state.item);

        task->state.status = TASK_STATUS_INACTIVE;

        break;

    }

}

unsigned int scheduler_rmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = task_rmessage(task, size, count, buffer);

    scheduler_setstatus(task, count ? TASK_STATUS_ACTIVE : TASK_STATUS_BLOCKED);

    return count;

}

unsigned int scheduler_wmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    count = task_wmessage(task, size, count, buffer);

    scheduler_setstatus(task, count ? TASK_STATUS_ACTIVE : TASK_STATUS_BLOCKED);

    return count;

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

