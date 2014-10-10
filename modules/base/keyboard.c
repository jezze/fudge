#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "keyboard.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_keyboard_node *node = (struct base_keyboard_node *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

void base_keyboard_registerinterface(struct base_keyboard_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_keyboard_registernode(struct base_keyboard_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void base_keyboard_unregisterinterface(struct base_keyboard_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_keyboard_unregisternode(struct base_keyboard_node *node)
{

}

void base_keyboard_initinterface(struct base_keyboard_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_keyboard_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_KEYBOARD, bus, id);

    interface->rdata = rdata;
    interface->wdata = wdata;

}

void base_keyboard_initnode(struct base_keyboard_node *node, struct base_keyboard_interface *interface)
{

    memory_clear(node, sizeof (struct base_keyboard_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->data.read = data_read;

}

void base_keyboard_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "keyboard");
    system_registernode(&root);

}

