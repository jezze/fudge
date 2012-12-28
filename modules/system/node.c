#include <fudge/memory.h>
#include "system.h"

void system_init_node(struct system_node *node, unsigned int type, char *name)
{

    memory_clear(node, sizeof (struct system_node));

    node->type = type;
    node->name = name;

}

