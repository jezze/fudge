#include <fudge.h>
#include <kernel.h>
#include <system/system.h>

static struct system_node root;

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_NORMAL | SYSTEM_NODETYPE_MULTI, "null");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

