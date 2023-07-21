#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "debug.h"
#include "task.h"

void task_signal(struct task *task, unsigned int signal)
{

    spinlock_acquire(&task->spinlock);

    switch (signal)
    {

    case TASK_SIGNAL_KILL:
        task->signals.kills++;

        break;

    case TASK_SIGNAL_BLOCK:
        task->signals.blocks++;

        break;

    case TASK_SIGNAL_UNBLOCK:
        task->signals.unblocks++;

        break;

    }

    spinlock_release(&task->spinlock);

}

unsigned int task_transition(struct task *task, unsigned int state)
{

    unsigned int valid = 0;

    spinlock_acquire(&task->spinlock);

    if (task->state != state)
    {

        switch (state)
        {

        case TASK_STATE_DEAD:
            if (task->state == TASK_STATE_NEW || task->state == TASK_STATE_UNBLOCKED || task->state == TASK_STATE_RUNNING)
                task->state = state;

            break;

        case TASK_STATE_NEW:
            if (task->state == TASK_STATE_DEAD)
                task->state = state;

            break;

        case TASK_STATE_BLOCKED:
            if (task->state == TASK_STATE_RUNNING)
                task->state = state;

            break;

        case TASK_STATE_UNBLOCKED:
            if (task->state == TASK_STATE_BLOCKED)
                task->state = state;

            break;

        case TASK_STATE_ASSIGNED:
            if (task->state == TASK_STATE_NEW || task->state == TASK_STATE_UNBLOCKED || task->state == TASK_STATE_RUNNING)
                task->state = state;

            break;

        case TASK_STATE_RUNNING:
            if (task->state == TASK_STATE_ASSIGNED)
                task->state = state;

            break;

        }

        valid = task->state == state;

    }

    spinlock_release(&task->spinlock);

    if (!valid)
        DEBUG_FMT0(DEBUG_ERROR, "state transition failed");

    return valid;

}

void task_register(struct task *task)
{

    resource_register(&task->resource);

}

void task_unregister(struct task *task)
{

    resource_unregister(&task->resource);

}

void task_resetsignals(struct task_signals *signals)
{

    signals->kills = 0;
    signals->blocks = 0;
    signals->unblocks = 0;

}

void task_resetthread(struct task_thread *thread)
{

    thread->ip = 0;
    thread->sp = 0;

}

void task_initsignals(struct task_signals *signals)
{

    signals->kills = 0;
    signals->blocks = 0;
    signals->unblocks = 0;

}

void task_initthread(struct task_thread *thread)
{

    thread->ip = 0;
    thread->sp = 0;

}

void task_reset(struct task *task)
{

    task_resetsignals(&task->signals);
    task_resetthread(&task->thread);

    task->state = TASK_STATE_DEAD;

}

void task_init(struct task *task, unsigned int id)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    task_initsignals(&task->signals);
    task_initthread(&task->thread);
    binary_initnode(&task->node);
    spinlock_init(&task->spinlock);

    task->id = id;
    task->state = TASK_STATE_DEAD;

}

