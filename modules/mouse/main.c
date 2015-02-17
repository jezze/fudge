#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "mouse.h"

static struct system_node root;

void mouse_notify(struct mouse_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->node.data.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_MOUSE, count, buffer);

}

void mouse_registerinterface(struct mouse_interface *interface, unsigned int id)
{

    base_registerinterface(&interface->base, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.data);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void mouse_initinterface(struct mouse_interface *interface, struct base_driver *driver)
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_MAILBOX, "data");

}

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "mouse");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

