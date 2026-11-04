#include <fudge.h>
#include "core.h"

void core_register(struct core *core)
{

    core->state = CORE_STATE_ACTIVE;

}

void core_init(struct core *core)
{

    list_init(&core->tasks);

    core->state = CORE_STATE_DEAD;
    core->itask = 0;

}

