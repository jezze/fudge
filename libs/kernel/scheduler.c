#include <fudge/kernel.h>
#include "vfs.h"
#include "task.h"

static struct list free;
static struct list used;
static struct list blocked;

void scheduler_block(struct task *task)
{

    list_move(&blocked, &used, &task->item);

}

void scheduler_unblock(struct task *task)
{

    list_move(&used, &blocked, &task->item);

}

void scheduler_use(struct task *task)
{

    list_move(&used, &free, &task->item);

}

void scheduler_unuse(struct task *task)
{

    list_move(&free, &used, &task->item);

}

void scheduler_add(struct task *task)
{

    list_add(&free, &task->item);

}

struct task *scheduler_find_used_task()
{

    if (!used.tail)
        return 0;

    return used.tail->data;

}

struct task *scheduler_find_free_task()
{

    if (!free.tail)
        return 0;

    return free.tail->data;

}

void scheduler_init()
{

    list_init(&free);
    list_init(&used);
    list_init(&blocked);

}

