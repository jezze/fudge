#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"

void task_init(struct task *task, unsigned long ip, unsigned long sp)
{

    unsigned int i;

    memory_clear(task, sizeof (struct task));
    list_inititem(&task->item, task);
    resource_init(&task->resource, RESOURCE_TYPE_TASK, task);

    for (i = 0; i < TASK_DESCRIPTORS; i++)
        vfs_initdescriptor(&task->descriptors[i]);

    task->registers.ip = ip;
    task->registers.sp = sp;

}

