#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>

static struct system_node root;

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_NORMAL, "null");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

