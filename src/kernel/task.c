#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"

void task_signal(struct task *task, unsigned int signal)
{

    spinlock_acquire(&task->spinlock);

    switch (signal)
    {

    case TASK_SIGNAL_KILL:
        task->sigkills++;

        break;

    case TASK_SIGNAL_BLOCK:
        task->sigblocks++;

        break;

    case TASK_SIGNAL_UNBLOCK:
        task->sigunblocks++;

        break;

    }

    spinlock_release(&task->spinlock);

}

void task_unsignal(struct task *task, unsigned int signal)
{

    spinlock_acquire(&task->spinlock);

    switch (signal)
    {

    case TASK_SIGNAL_KILL:
        task->sigkills = 0;

        break;

    case TASK_SIGNAL_BLOCK:
        task->sigblocks = 0;

        break;

    case TASK_SIGNAL_UNBLOCK:
        task->sigunblocks = 0;

        break;

    }

    spinlock_release(&task->spinlock);

}

void task_transition(struct task *task, unsigned int state)
{

    spinlock_acquire(&task->spinlock);

    switch (state)
    {

    case TASK_STATE_KILLED:
        task->state = state;

        break;

    case TASK_STATE_ASSIGNED:
        task->state = state;

        break;

    case TASK_STATE_BLOCKED:
        if (task->state == TASK_STATE_RUNNING)
            task->state = state;

        break;

    case TASK_STATE_RUNNING:
        if (task->state == TASK_STATE_ASSIGNED || task->state == TASK_STATE_BLOCKED)
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

void task_resetthread(struct task_thread *thread)
{

    thread->ip = 0;
    thread->sp = 0;

}

void task_initthread(struct task_thread *thread)
{

    thread->ip = 0;
    thread->sp = 0;

}

void task_reset(struct task *task)
{

    task_resetthread(&task->thread);

    task->state = TASK_STATE_KILLED;
    task->sigkills = 0;
    task->sigblocks = 0;
    task->sigunblocks = 0;

}

void task_init(struct task *task, unsigned int id)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    task_initthread(&task->thread);
    binary_initnode(&task->node);
    spinlock_init(&task->spinlock);

    task->format = 0;
    task->id = id;
    task->state = TASK_STATE_KILLED;
    task->sigkills = 0;
    task->sigblocks = 0;
    task->sigunblocks = 0;

}

