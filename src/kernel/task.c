#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "scheduler.h"
#include "task.h"

unsigned int task_rmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    return buffer_rcfifo(&task->mailbox.buffer, size * count, buffer);

}

unsigned int task_wmessage(struct task *task, unsigned int size, unsigned int count, void *buffer)
{

    return buffer_wcfifo(&task->mailbox.buffer, size * count, buffer);

}

void task_init(struct task *task)
{

    unsigned int i;

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->state.item, task);
    list_inititem(&task->mailbox.item, task);
    buffer_init(&task->mailbox.buffer, TASK_MAILBOXSIZE, task->mailbox.data);

    for (i = 0; i < TASK_DESCRIPTORS; i++)
        vfs_initdescriptor(&task->descriptors[i]);

}

