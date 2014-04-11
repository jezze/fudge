#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"

void task_init(struct task *task, unsigned long ip, unsigned long sp)
{

    unsigned int i;

    memory_clear(task, sizeof (struct task));
    list_init_item(&task->item, task);
    resource_init_item(&task->resource, TASK_RESOURCE, task);

    task->registers.ip = ip;
    task->registers.sp = sp;

    for (i = 0; i < TASK_DESCRIPTORS; i++)
        vfs_init_descriptor(&task->descriptors[i], 0, 0);

}

