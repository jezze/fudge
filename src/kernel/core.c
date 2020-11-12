#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"

struct task *core_schedule(struct core *core)
{

    struct list_item *current = list_picktail(&core->tasks);

    return (current) ? current->data : 0;

}

struct task *core_unschedule(struct core *core)
{

    if (core->task)
    {

        /* Task could be blocked */
        if (core->task->item.list)
            return 0;

        list_add(&core->tasks, &core->task->item);

    }

    return 0;

}

void core_register(struct core *core)
{

    resource_register(&core->resource);

}

void core_unregister(struct core *core)
{

    resource_unregister(&core->resource);

}

void core_init(struct core *core, unsigned int id, unsigned int sp, struct task *task)
{

    list_inititem(&core->item, core);
    resource_init(&core->resource, RESOURCE_CORE, core);

    core->id = id;
    core->sp = sp;
    core->task = task;

}

