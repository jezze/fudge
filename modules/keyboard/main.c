#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include <event/event.h>
#include "keyboard.h"

static struct system_node root;

void keyboard_notify(struct keyboard_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->data.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_KEYBOARD, count, buffer);

}

void keyboard_registerinterface(struct keyboard_interface *interface, unsigned int id)
{

    system_registerinterface(&interface->base, id);
    system_addchild(&interface->base.root, &interface->data);
    system_addchild(&root, &interface->base.root);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    system_unregisterinterface(&interface->base);
    system_removechild(&interface->base.root, &interface->data);
    system_removechild(&root, &interface->base.root);

}

void keyboard_initinterface(struct keyboard_interface *interface, struct base_driver *driver)
{

    system_initinterface(&interface->base, driver);
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");

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

