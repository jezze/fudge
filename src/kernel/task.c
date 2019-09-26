#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"

void task_initthread(struct task_thread *thread)
{

}

void task_init(struct task *task, unsigned int id)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->item, task);
    task_initthread(&task->thread);

    task->id = id;

}

