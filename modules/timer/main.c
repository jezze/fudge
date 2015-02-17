#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include "timer.h"

static struct system_node root;

void timer_notify(struct timer_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->mailboxes, count, buffer);

}

static unsigned int interfacenode_sleepopen(struct system_node *self)
{

    struct timer_interfacenode *node = (struct timer_interfacenode *)self->parent;

    scheduler_mailboxes_addactive(&node->interface->mailboxes);

    return system_open(self);

}

static unsigned int interfacenode_sleepclose(struct system_node *self)
{

    struct timer_interfacenode *node = (struct timer_interfacenode *)self->parent;

    scheduler_mailboxes_removeactive(&node->interface->mailboxes);

    return system_close(self);

}

static unsigned int interfacenode_sleepread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct timer_interfacenode *node = (struct timer_interfacenode *)self->parent;

    return scheduler_mailboxes_readactive(&node->interface->mailboxes, count, buffer);

}

void timer_registerinterface(struct timer_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
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
    list_init(&interface->mailboxes);

    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.sleep, SYSTEM_NODETYPE_NORMAL, "sleep");

    interface->node.interface = interface;
    interface->node.sleep.open = interfacenode_sleepopen;
    interface->node.sleep.close = interfacenode_sleepclose;
    interface->node.sleep.read = interfacenode_sleepread;

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

