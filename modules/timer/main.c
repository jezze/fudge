#include <fudge.h>
#include <kernel.h>
#include <base/base.h>
#include <system/system.h>
#include "timer.h"

static struct system_node root;

void timer_notify(struct timer_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->sleep.mailboxes, count, buffer);

}

void timer_registerinterface(struct timer_interface *interface, unsigned int id)
{

    system_registerinterface(&interface->base, id);
    system_addchild(&interface->base.root, &interface->sleep);
    system_addchild(&root, &interface->base.root);

}

void timer_unregisterinterface(struct timer_interface *interface)
{

    system_unregisterinterface(&interface->base);
    system_removechild(&interface->base.root, &interface->sleep);
    system_removechild(&root, &interface->base.root);

}

void timer_initinterface(struct timer_interface *interface, struct base_driver *driver)
{

    system_initinterface(&interface->base, driver);
    system_initnode(&interface->sleep, SYSTEM_NODETYPE_MAILBOX, "sleep");

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

