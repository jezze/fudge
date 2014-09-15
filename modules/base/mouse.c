#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "mouse.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_mouse_node *node = (struct base_mouse_node *)self->parent;

    return node->interface->rdata(node->device->bus, node->device->id, offset, count, buffer);

}

void base_mouse_registerinterface(struct base_mouse_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_mouse_registernode(struct base_mouse_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void base_mouse_unregisterinterface(struct base_mouse_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_mouse_unregisternode(struct base_mouse_node *node)
{

}

void base_mouse_initinterface(struct base_mouse_interface *interface, unsigned int (*rdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_mouse_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_MOUSE);

    interface->rdata = rdata;

}

void base_mouse_initnode(struct base_mouse_node *node, struct base_device *device, struct base_mouse_interface *interface)
{

    memory_clear(node, sizeof (struct base_mouse_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, device->bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->device = device;
    node->interface = interface;
    node->data.read = data_read;

}

void base_mouse_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "mouse");
    system_registernode(&root);

}

