#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "keyboard.h"

static struct system_node root;

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_interfacenode *node = (struct keyboard_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

void keyboard_registerinterface(struct keyboard_interface *interface)
{

    base_registerinterface(&interface->base);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.data);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void keyboard_initinterface(struct keyboard_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct keyboard_interface));
    base_initinterface(&interface->base, driver, bus, id);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->rdata = rdata;
    interface->node.interface = interface;
    interface->node.data.read = interfacenode_dataread;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "keyboard");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

