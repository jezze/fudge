#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "mouse.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct mouse_interfacenode *node = (struct mouse_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

void mouse_registerinterface(struct mouse_interface *interface)
{

    base_registerinterface(&interface->base);

}

void mouse_registerinterfacenode(struct mouse_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void mouse_unregisterinterfacenode(struct mouse_interfacenode *node)
{

    system_removechild(&node->base, &node->data);
    system_removechild(&root, &node->base);

}

void mouse_initinterface(struct mouse_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct mouse_interface));
    base_initinterface(&interface->base, bus, id);

    interface->rdata = rdata;

}

void mouse_initinterfacenode(struct mouse_interfacenode *node, struct mouse_interface *interface)
{

    memory_clear(node, sizeof (struct mouse_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->data.read = data_read;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "mouse");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

