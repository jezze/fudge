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

void task_transition(struct task *task, unsigned int state)
{

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

void task_checksignals(struct task *task)
{

    if (task->signals.kills)
        task_transition(task, TASK_STATE_DEAD);
    else if (task->signals.blocks)
        task_transition(task, TASK_STATE_BLOCKED);
    else if (task->signals.unblocks)
        task_transition(task, TASK_STATE_UNBLOCKED);

    task_resetsignals(&task->signals);

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

void task_resetmailboxes(struct task *task)
{

    unsigned int i;

    for (i = 0; i < TASK_MAILBOXES; i++)
        task->imailbox[i] = 0;

}

void task_reset(struct task *task)
{

    task_resetsignals(&task->signals);
    task_resetthread(&task->thread);
    task_resetmailboxes(task);

    task->state = TASK_STATE_DEAD;
    task->address = 0;

}

void task_init(struct task *task)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    spinlock_init(&task->spinlock);
    task_reset(task);

}

