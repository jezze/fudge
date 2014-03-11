#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"

void task_init(struct task *task, unsigned long ip, unsigned long sp)
{

    memory_clear(task, sizeof (struct task));
    list_init_item(&task->item, task);
    resource_init_item(&task->resource, RESOURCE_TYPE_TASK, task);

    task->registers.ip = ip;
    task->registers.sp = sp;

}

