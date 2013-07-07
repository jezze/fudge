#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

struct task_descriptor *task_get_descriptor(struct task *task, unsigned int index)
{

    if (!index || index >= TASK_DESCRIPTORS)
        return 0;

    return &task->descriptors[index];

}

void task_init(struct task *task, unsigned int ip, unsigned int sp, unsigned int fp)
{

    memory_clear(task, sizeof (struct task));

    task->state = TASK_STATE_USED;
    task->registers.ip = ip;
    task->registers.sp = sp;
    task->registers.fp = fp;

}

