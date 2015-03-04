#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"

void task_copy(struct task *task, struct task *next)
{

    unsigned int i;

    for (i = 0x00; i < 0x08; i++)
    {

        next->descriptors[i + 0x00].channel = task->descriptors[i + 0x08].channel;
        next->descriptors[i + 0x00].id = task->descriptors[i + 0x08].id;
        next->descriptors[i + 0x08].channel = task->descriptors[i + 0x08].channel;
        next->descriptors[i + 0x08].id = task->descriptors[i + 0x08].id;
        next->descriptors[i + 0x10].channel = 0;
        next->descriptors[i + 0x10].id = 0;
        next->descriptors[i + 0x18].channel = 0;
        next->descriptors[i + 0x18].id = 0;

    }

}

void task_init(struct task *task)
{

    unsigned int i;

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->state.item, task);
    list_inititem(&task->mailbox.item, task);
    buffer_init(&task->mailbox.buffer, 1, TASK_MAILBOXSIZE, task->mailbox.data);

    for (i = 0; i < TASK_DESCRIPTORS; i++)
        vfs_initdescriptor(&task->descriptors[i]);

}

