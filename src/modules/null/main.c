#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node root;

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_NORMAL, "null");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

