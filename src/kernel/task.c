#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "container.h"
#include "task.h"
#include "service.h"

void task_setstate(struct task *task, unsigned int ip, unsigned int sp)
{

    task->state.ip = ip;
    task->state.sp = sp;
    task->state.rewind = 0;

}

void task_init(struct task *task, unsigned int id)
{

    resource_init(&task->resource, RESOURCE_TASK, task);
    list_inititem(&task->state.item, task);
    ring_init(&task->mailbox, TASK_MAILBOXSIZE, task->mailboxdata);

    task->id = id;

}

