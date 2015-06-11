#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"

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

