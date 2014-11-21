#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "terminal.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_interfacenode *node = (struct terminal_interfacenode *)self->parent;
 
    return node->interface->rdata(offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct terminal_interfacenode *node = (struct terminal_interfacenode *)self->parent;

    return node->interface->wdata(offset, count, buffer);

}

void terminal_registerinterface(struct terminal_interface *interface)
{

    base_registerinterface(&interface->base);

}

void terminal_registerinterfacenode(struct terminal_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void terminal_unregisterinterface(struct terminal_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void terminal_unregisterinterfacenode(struct terminal_interfacenode *node)
{

    system_removechild(&node->base, &node->data);
    system_removechild(&root, &node->base);

}

void terminal_initinterface(struct terminal_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct terminal_interface));
    base_initinterface(&interface->base, driver, bus, id);

    interface->rdata = rdata;
    interface->wdata = wdata;

}

void terminal_initinterfacenode(struct terminal_interfacenode *node, struct terminal_interface *interface)
{

    memory_clear(node, sizeof (struct terminal_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->data.read = data_read;
    node->data.write = data_write;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "terminal");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

