#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"

void core_sorttasks(struct core *core)
{

    struct list normal;
    struct list kicked;
    struct task *task;

    list_init(&normal);
    list_init(&kicked);

    while ((task = list_pickhead(&core->tasks)))
    {

        spinlock_acquire(&task->spinlock);

        if (task->kicked)
            list_add(&kicked, &task->item);
        else
            list_add(&normal, &task->item);

        task->kicked = 0;

        spinlock_release(&task->spinlock);

    }

    while ((task = list_pickhead(&normal)))
        list_add(&core->tasks, &task->item);

    while ((task = list_pickhead(&kicked)))
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

}

