#include <fudge.h>
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

void task_register(struct task *task, unsigned long mmap, unsigned long buffer, unsigned long ip, unsigned long sp)
{

    task->mmap = mmap;
    task->buffer = buffer;
    task->thread.ip = ip;
    task->thread.sp = sp;

}

void task_reset(struct task *task)
{

    unsigned int i;

    task->state = TASK_STATE_DEAD;
    task->waitingfor = 0;
    task->aborted = 0;
    task->started = 0;
    task->mmap = 0;
    task->buffer = 0;
    task->signals.kill = 0;
    task->signals.block = 0;
    task->signals.unblock = 0;
    task->thread.ip = 0;
    task->thread.sp = 0;
    task->start.ip = 0;
    task->start.sp = 0;

    for (i = 0; i < TASK_MAILBOXES; i++)
        task->imailbox[i] = 0;

}

void task_init(struct task *task)
{

    spinlock_init(&task->spinlock);
    task_reset(task);

}

