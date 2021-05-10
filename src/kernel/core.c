#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "core.h"

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

