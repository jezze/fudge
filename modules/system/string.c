#include <fudge/memory.h>
#include "system.h"

void system_init_string(struct system_string *string, char *name, char *value)
{

    memory_clear(string, sizeof (struct system_string));

    system_init_node(&string->base, SYSTEM_NODE_TYPE_STRING, name);

    string->value = value;

}

