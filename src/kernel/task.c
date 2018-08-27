#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"

unsigned int task_read(struct task *task, void *buffer, unsigned int count)
{

    spinlock_acquire(&task->mailbox.spinlock);

    count = ring_read(&task->mailbox.ring, buffer, count);

    spinlock_release(&task->mailbox.spinlock);

    return count;

}

unsigned int task_writeall(struct task *task, void *buffer, unsigned int count)
{

    spinlock_acquire(&task->mailbox.spinlock);

    count = ring_writeall(&task->mailbox.ring, buffer, count);

    spinlock_release(&task->mailbox.spinlock);

    return count;

}

void task_initthread(struct task_thread *thread)
{

}

void task_initmailbox(struct task_mailbox *mailbox, char *buffer)
{

    ring_init(&mailbox->ring, TASK_MAILBOXSIZE, buffer);

}

void task_init(struct task *task, unsigned int id, char *buffer)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->item, task);
    task_initthread(&task->thread);
    task_initmailbox(&task->mailbox, buffer);

    task->id = id;

}

