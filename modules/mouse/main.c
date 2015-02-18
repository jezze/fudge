#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <event/event.h>
#include "mouse.h"

static struct system_node root;

void mouse_notify(struct mouse_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->data.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_MOUSE, count, buffer);

}

void mouse_registerinterface(struct mouse_interface *interface, unsigned int id)
{

    system_registerinterface(&interface->base, id);
    system_addchild(&interface->base.root, &interface->data);
    system_addchild(&root, &interface->base.root);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    system_unregisterinterface(&interface->base);
    system_removechild(&interface->base.root, &interface->data);
    system_removechild(&root, &interface->base.root);

}

void mouse_initinterface(struct mouse_interface *interface, struct base_driver *driver)
{

    system_initinterface(&interface->base, driver);
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");

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

