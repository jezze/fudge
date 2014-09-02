#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "block.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_block_node *node = (struct base_block_node *)self->parent;

    return node->interface->rdata(node->device->bus, node->device->id, offset, count, buffer);

}

void base_block_registerinterface(struct base_block_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_block_registerprotocol(struct base_block_protocol *protocol)
{

}

void base_block_registernode(struct base_block_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void base_block_unregisterinterface(struct base_block_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_block_unregisterprotocol(struct base_block_protocol *protocol)
{

}

void base_block_unregisternode(struct base_block_node *node)
{

}

void base_block_initinterface(struct base_block_interface *interface, unsigned int (*rdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(struct base_bus *bus, unsigned int id, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_block_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_BLOCK);

    interface->rdata = rdata;
    interface->wdata = wdata;

}

void base_block_initprotocol(struct base_block_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct base_block_protocol));

    protocol->name = name;

}

void base_block_initnode(struct base_block_node *node, struct base_device *device, struct base_block_interface *interface)
{

    memory_clear(node, sizeof (struct base_block_node));
    system_initmultigroup(&node->base, device->bus->name);
    system_initstream(&node->data, "data");

    node->device = device;
    node->interface = interface;
    node->data.read = data_read;

}

void base_block_setup()
{

    system_initgroup(&root, "block");
    system_registernode(&root);

}

