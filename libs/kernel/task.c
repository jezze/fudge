#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

static struct list free;
static struct list used;
static struct list blocked;

void task_sched_block(struct task *task)
{

    list_move(&blocked, &used, &task->item);

}

void task_sched_unblock(struct task *task)
{

    list_move(&used, &blocked, &task->item);

}

void task_sched_use(struct task *task)
{

    list_move(&used, &free, &task->item);

}

void task_sched_unuse(struct task *task)
{

    list_move(&free, &used, &task->item);

}

void task_sched_add(struct task *task)
{

    list_add(&free, &task->item);

}

struct task *task_sched_find_used_task()
{

    if (!used.tail)
        return 0;

    return used.tail->data;

}

struct task *task_sched_find_free_task()
{

    if (!free.tail)
        return 0;

    return free.tail->data;

}

void task_sched_init()
{

    list_init(&free);
    list_init(&used);
    list_init(&blocked);

}

void task_init(struct task *task, unsigned long ip, unsigned long sp)
{

    memory_clear(task, sizeof (struct task));
    list_init_item(&task->item, task);

    task->registers.ip = ip;
    task->registers.sp = sp;

}

