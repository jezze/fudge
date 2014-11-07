#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "block.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_block_interfacenode *node = (struct base_block_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

void base_block_registerinterface(struct base_block_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_block_registerinterfacenode(struct base_block_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void base_block_unregisterinterface(struct base_block_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_block_unregisterinterfacenode(struct base_block_interfacenode *node)
{

    system_removechild(&node->base, &node->data);
    system_removechild(&root, &node->base);

}

void base_block_initinterface(struct base_block_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_block_interface));
    base_initinterface(&interface->base, bus, id);

    interface->rdata = rdata;
    interface->wdata = wdata;

}

void base_block_initinterfacenode(struct base_block_interfacenode *node, struct base_block_interface *interface)
{

    memory_clear(node, sizeof (struct base_block_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->data.read = data_read;

}

void base_block_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "block");
    system_registernode(&root);

}

