#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

void task_init(struct task *task, enum task_state state, unsigned int ip, unsigned int sp)
{

    memory_clear(task, sizeof (struct task));

    task->state = state;
    task->registers.ip = ip;
    task->registers.sp = sp;

}

