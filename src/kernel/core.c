#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"

void core_sorttasks(struct core *core)
{

    struct list normal;
    struct list unblocked;
    struct task *task;

    list_init(&normal);
    list_init(&unblocked);

    while ((task = list_pickhead(&core->tasks)))
    {

        spinlock_acquire(&task->spinlock);

        if (task->unblocks)
            list_add(&unblocked, &task->item);
        else
            list_add(&normal, &task->item);

        task->unblocks = 0;

        spinlock_release(&task->spinlock);

    }

    while ((task = list_pickhead(&normal)))
        list_add(&core->tasks, &task->item);

    while ((task = list_pickhead(&unblocked)))
        list_add(&core->tasks, &task->item);

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
    core->task = 0;

}

