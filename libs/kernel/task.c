#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

void task_init(struct task *task, unsigned int ip, unsigned int sp)
{

    memory_clear(task, sizeof (struct task));

    task->state = TASK_STATE_USED;
    task->registers.ip = ip;
    task->registers.sp = sp;

}

