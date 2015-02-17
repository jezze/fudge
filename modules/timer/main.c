#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include "timer.h"

static struct system_node root;

void timer_notify(struct timer_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->node.sleep.mailboxes, count, buffer);

}

void timer_registerinterface(struct timer_interface *interface, unsigned int id)
{

    base_registerinterface(&interface->base, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.sleep);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.sleep);
    system_removechild(&root, &interface->node.base);

}

void timer_initinterface(struct timer_interface *interface, struct base_driver *driver)
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.sleep, SYSTEM_NODETYPE_MAILBOX, "sleep");

}

void module_init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "timer");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

