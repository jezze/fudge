#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "keyboard.h"

static struct system_node root;

void keyboard_notify(struct keyboard_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->node.data.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_KEYBOARD, count, buffer);

}

void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id)
{

    base_registerinterface(&interface->base, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.data);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void keyboard_initinterface(struct keyboard_interface *interface, struct base_driver *driver)
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_MAILBOX, "data");

}

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "keyboard");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

