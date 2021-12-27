#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"

void task_setstate(struct task *task, unsigned int state)
{

    spinlock_acquire(&task->spinlock);

    switch (state)
    {

    case TASK_STATE_NORMAL:
        task->state = state;

        break;

    case TASK_STATE_KILLED:
        task->state = state;

        break;

    case TASK_STATE_BLOCKED:
        if (task->state == TASK_STATE_NORMAL)
            task->state = state;

        break;

    }

    spinlock_release(&task->spinlock);

}

void task_register(struct task *task)
{

    resource_register(&task->resource);

}

void task_unregister(struct task *task)
{

    resource_unregister(&task->resource);

}

void task_initthread(struct task_thread *thread)
{

    thread->ip = 0;
    thread->sp = 0;

}

void task_init(struct task *task, unsigned int id)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->item, task);
    task_initthread(&task->thread);
    binary_initnode(&task->node);
    spinlock_init(&task->spinlock);

    task->format = 0;
    task->id = id;
    task->state = TASK_STATE_NORMAL;
    task->kicked = 0;

}

