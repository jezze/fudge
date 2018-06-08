#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "console.h"

static struct system_node root;

void console_notify(struct console_interface *interface, void *buffer, unsigned int count)
{

    kernel_multicast(&interface->idata.states, buffer, count);

}

void console_registerinterface(struct console_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->idata);
    system_addchild(&interface->root, &interface->odata);
    system_addchild(&root, &interface->root);

}

void console_unregisterinterface(struct console_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->idata);
    system_removechild(&interface->root, &interface->odata);
    system_removechild(&root, &interface->root);

}

void console_initinterface(struct console_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_CONSOLEINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->idata, SYSTEM_NODETYPE_MAILBOX, "idata");
    system_initnode(&interface->odata, SYSTEM_NODETYPE_NORMAL, "odata");

    interface->id = id;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

