#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

void task_init(struct task *task, unsigned long ip, unsigned long sp)
{

    memory_clear(task, sizeof (struct task));
    list_init_item(&task->item, task);

    task->registers.ip = ip;
    task->registers.sp = sp;

}

