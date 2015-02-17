#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "mouse.h"

static struct system_node root;

void mouse_notify(struct mouse_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&interface->mailboxes, count, buffer);
    event_notify(EVENT_TYPE_MOUSE, count, buffer);

}

static unsigned int interfacenode_dataopen(struct system_node *self)
{

    struct mouse_interfacenode *node = (struct mouse_interfacenode *)self->parent;

    scheduler_mailboxes_addactive(&node->interface->mailboxes);

    return system_open(self);

}

static unsigned int interfacenode_dataclose(struct system_node *self)
{

    struct mouse_interfacenode *node = (struct mouse_interfacenode *)self->parent;

    scheduler_mailboxes_removeactive(&node->interface->mailboxes);

    return system_close(self);

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct mouse_interfacenode *node = (struct mouse_interfacenode *)self->parent;

    return scheduler_mailboxes_readactive(&node->interface->mailboxes, count, buffer);

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
    list_init(&interface->mailboxes);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->node.interface = interface;
    interface->node.data.open = interfacenode_dataopen;
    interface->node.data.close = interfacenode_dataclose;
    interface->node.data.read = interfacenode_dataread;

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

