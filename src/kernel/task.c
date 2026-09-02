#include <fudge.h>
#include "resource.h"
#include "mmap.h"
#include "binary.h"
#include "debug.h"
#include "task.h"

void task_signal(struct task *task, unsigned int signal)
{

    switch (signal)
    {

    case TASK_SIGNAL_KILL:
        task->signals.kill++;

        break;

    case TASK_SIGNAL_BLOCK:
        task->signals.block++;

        break;

    case TASK_SIGNAL_UNBLOCK:
        task->signals.unblock++;

        break;

    }

}

unsigned int task_transition(struct task *task, unsigned int state)
{

    if (task->state != state)
    {

        switch (state)
        {

        case TASK_STATE_DEAD:
            if (task->state == TASK_STATE_NEW || task->state == TASK_STATE_RUNNING || task->state == TASK_STATE_UNBLOCKED)
            {

                task->state = state;
                task->signals.kill = 0;
                task->signals.unblock = 0;
                task->signals.block = 0;

                return state;

            }

            break;

        case TASK_STATE_UNBLOCKED:
            if (task->state == TASK_STATE_BLOCKED)
            {

                task->state = state;
                task->signals.unblock = 0;
                task->signals.block = 0;

                return state;

            }

            break;

        case TASK_STATE_BLOCKED:
            if (task->state == TASK_STATE_RUNNING)
            {

                task->state = state;
                task->signals.unblock = 0;
                task->signals.block = 0;

                return state;

            }

            break;

        case TASK_STATE_NEW:
            if (task->state == TASK_STATE_DEAD)
            {

                task->state = state;

                return state;

            }

            break;

        case TASK_STATE_ASSIGNED:
            if (task->state == TASK_STATE_NEW || task->state == TASK_STATE_RUNNING || task->state == TASK_STATE_UNBLOCKED)
            {

                task->state = state;

                return state;

            }

            break;

        case TASK_STATE_RUNNING:
            if (task->state == TASK_STATE_ASSIGNED)
            {

                task->state = state;

                return state;

            }

            break;

        }

    }

    return 0;

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

    signals->kill = 0;
    signals->block = 0;
    signals->unblock = 0;

}

void task_resetthread(struct task_thread *thread, unsigned long ip, unsigned long sp)
{

    thread->ip = ip;
    thread->sp = sp;

}

void task_resetmailboxes(struct task *task)
{

    unsigned int i;

    for (i = 0; i < TASK_MAILBOXES; i++)
        task->imailbox[i] = 0;

}

void task_reset(struct task *task, unsigned long address, unsigned long mmap)
{

    task_resetsignals(&task->signals);
    task_resetthread(&task->thread, 0, 0);
    task_resetmailboxes(task);

    task->state = TASK_STATE_DEAD;
    task->address = address;
    task->mmap = mmap;

}

void task_init(struct task *task)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    spinlock_init(&task->spinlock);
    task_reset(task, 0, 0);

}

