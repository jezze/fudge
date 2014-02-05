#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

static struct list free;
static struct list used;
static struct list blocked;

void task_sched_block(struct task *task)
{

    list_remove(&used, &task->item);
    list_add(&blocked, &task->item);

}

void task_sched_unblock(struct task *task)
{

    list_remove(&blocked, &task->item);
    list_add(&used, &task->item);

}

void task_sched_use(struct task *task)
{

    list_remove(&free, &task->item);
    list_add(&used, &task->item);

}

void task_sched_unuse(struct task *task)
{

    list_remove(&used, &task->item);
    list_add(&free, &task->item);

}

void task_sched_add(struct task *task)
{

    list_add(&free, &task->item);

}

struct task *task_sched_find_next_task()
{

    if (!used.tail)
        return 0;

    return used.tail->self;

}

struct task *task_sched_find_free_task()
{

    if (!free.head)
        return 0;

    return free.head->self;

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

