#include <kernel.h>
#include "resource.h"
#include "vfs.h"
#include "task.h"

static struct list containerlist;
static struct list tasklist;
static struct resource_item containers;
static struct resource_item tasks;
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

void scheduler_register_task(struct task *task)
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

void scheduler_setup()
{

    list_init(&free);
    list_init(&used);
    list_init(&blocked);
    list_init(&containerlist);
    list_init(&tasklist);
    resource_init_item(&containers, &containerlist, RESOURCE_TYPE_CONTAINER, 10, "containers");
    resource_register_item(&containers);
    resource_init_item(&tasks, &tasklist, RESOURCE_TYPE_TASK, 5, "tasks");
    resource_register_item(&tasks);

}

void scheduler_register_container(struct resource_item *item)
{

    list_add(&containerlist, &item->item);

}

void scheduler_register_task2(struct resource_item *item)
{

    list_add(&tasklist, &item->item);

}

