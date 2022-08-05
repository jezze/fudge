#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"

void core_sorttasks(struct core *core)
{

    struct list normal;
    struct list unblocked;
    struct list_item *taskitem;

    list_init(&normal);
    list_init(&unblocked);

    while ((taskitem = list_pickhead(&core->tasks)))
    {

        struct task *task = taskitem->data;

        spinlock_acquire(&task->spinlock);

        if (task->unblocks)
            list_add(&unblocked, taskitem);
        else
            list_add(&normal, taskitem);

        task->unblocks = 0;

        spinlock_release(&task->spinlock);

    }

    while ((taskitem = list_pickhead(&normal)))
        list_add(&core->tasks, taskitem);

    while ((taskitem = list_pickhead(&unblocked)))
        list_add(&core->tasks, taskitem);

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

