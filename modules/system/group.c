#include <fudge/memory.h>
#include "system.h"

void system_init_group(struct system_group *group, char *name)
{

    memory_clear(group, sizeof (struct system_group));

    system_init_node(&group->base, SYSTEM_NODE_TYPE_GROUP, name);

}

