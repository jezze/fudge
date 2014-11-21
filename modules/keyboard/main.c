#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "keyboard.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct keyboard_interfacenode *node = (struct keyboard_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

void keyboard_registerinterface(struct keyboard_interface *interface)
{

    base_registerinterface(&interface->base);

}

void keyboard_registerinterfacenode(struct keyboard_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void keyboard_unregisterinterface(struct keyboard_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void keyboard_unregisterinterfacenode(struct keyboard_interfacenode *node)
{

    system_removechild(&node->base, &node->data);
    system_removechild(&root, &node->base);

}

void keyboard_initinterface(struct keyboard_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct keyboard_interface));
    base_initinterface(&interface->base, bus, id);

    interface->rdata = rdata;

}

void keyboard_initinterfacenode(struct keyboard_interfacenode *node, struct keyboard_interface *interface)
{

    memory_clear(node, sizeof (struct keyboard_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->data.read = data_read;

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

