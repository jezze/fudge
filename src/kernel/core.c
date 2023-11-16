#include <fudge.h>
#include "resource.h"
#include "core.h"

void core_migrate(struct core *core, struct core *from)
{

    struct list_item *taskitem;

    while ((taskitem = list_pickhead(&from->tasks)))
        list_add(&core->tasks, taskitem);

    core->itask = from->itask;
    from->itask = 0;

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
    list_init(&core->tasks);

    core->id = id;
    core->sp = sp;
    core->itask = 0;

}

