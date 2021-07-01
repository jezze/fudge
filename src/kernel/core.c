#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"

void core_sorttasks(struct core *core)
{

    struct list_item *current;
    struct list normal;
    struct list kicked;

    list_init(&normal);
    list_init(&kicked);

    while ((current = list_pickhead(&core->tasks)))
    {

        struct task *task = current->data;

        spinlock_acquire(&task->spinlock);

        if (task->kicked)
            list_add(&kicked, &task->item);
        else
            list_add(&normal, &task->item);

        task->kicked = 0;

        spinlock_release(&task->spinlock);

    }

    while ((current = list_pickhead(&normal)))
    {

        struct task *task = current->data;

        list_add(&core->tasks, &task->item);

    }

    while ((current = list_pickhead(&kicked)))
    {

        struct task *task = current->data;

        list_add(&core->tasks, &task->item);

    }

}

void core_register(struct core *core)
{

    resource_register(&core->resource);

}

void core_unregister(struct core *core)
{

    resource_unregister(&core->resource);

}

void core_init(struct core *core, unsigned int id, unsigned int sp)
{

    resource_init(&core->resource, RESOURCE_CORE, core);
    list_inititem(&core->item, core);
    list_init(&core->tasks);

    core->id = id;
    core->sp = sp;

}

