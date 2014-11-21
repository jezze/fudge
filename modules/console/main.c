#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "console.h"

static struct system_node root;

static unsigned int ctrl_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->ctrl, sizeof (struct ctrl_consolectrl), offset);

}

static unsigned int ctrl_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;
 
    return node->interface->rdata(offset, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return node->interface->wdata(offset, count, buffer);

}

void console_registerinterface(struct console_interface *interface)
{

    base_registerinterface(&interface->base);

}

void console_registerinterfacenode(struct console_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->ctrl);
    system_addchild(&node->base, &node->data);

}

void console_unregisterinterface(struct console_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void console_unregisterinterfacenode(struct console_interfacenode *node)
{

    system_removechild(&node->base, &node->ctrl);
    system_removechild(&node->base, &node->data);
    system_removechild(&root, &node->base);

}

void console_initinterface(struct console_interface *interface, struct base_driver *driver, struct base_bus *bus, unsigned int id, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct console_interface));
    base_initinterface(&interface->base, driver, bus, id);
    ctrl_init_consolectrl(&interface->ctrl);

    interface->rdata = rdata;
    interface->wdata = wdata;

}

void console_initinterfacenode(struct console_interfacenode *node, struct console_interface *interface)
{

    memory_clear(node, sizeof (struct console_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->ctrl.read = ctrl_read;
    node->ctrl.write = ctrl_write;
    node->data.read = data_read;
    node->data.write = data_write;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "console");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

