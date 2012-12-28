#include <fudge/memory.h>
#include "system.h"

void system_init_integer(struct system_integer *integer, char *name, int value)
{

    memory_clear(integer, sizeof (struct system_integer));

    system_init_node(&integer->base, SYSTEM_NODE_TYPE_INTEGER, name);

    integer->value = value;

}

