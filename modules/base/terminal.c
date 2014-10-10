#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "terminal.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_terminal_node *node = (struct base_terminal_node *)self->parent;
 
    return node->interface->rdata(offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_terminal_node *node = (struct base_terminal_node *)self->parent;

    return node->interface->wdata(offset, count, buffer);

}

void base_terminal_registerinterface(struct base_terminal_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_terminal_registernode(struct base_terminal_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void base_terminal_unregisterinterface(struct base_terminal_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_terminal_unregisternode(struct base_terminal_node *node)
{

}

void base_terminal_initinterface(struct base_terminal_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_terminal_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_TERMINAL, bus, id);

    interface->rdata = rdata;
    interface->wdata = wdata;

}

void base_terminal_initnode(struct base_terminal_node *node, struct base_terminal_interface *interface)
{

    memory_clear(node, sizeof (struct base_terminal_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->data.read = data_read;
    node->data.write = data_write;

}

void base_terminal_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "terminal");
    system_registernode(&root);

}

