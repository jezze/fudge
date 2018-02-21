#include <fudge.h>
#include "resource.h"
#include "core.h"

void core_init(struct core *core, unsigned int id, unsigned int sp, struct task *task)
{

    list_inititem(&core->item, core);
    resource_init(&core->resource, RESOURCE_CORE, core);

    core->id = id;
    core->sp = sp;
    core->task = task;

}

