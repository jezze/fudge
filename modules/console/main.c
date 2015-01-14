#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "console.h"

static struct system_node root;

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->settings, sizeof (struct ctrl_consolesettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return memory_write(&node->interface->settings, sizeof (struct ctrl_consolesettings), buffer, count, offset);

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;
 
    return node->interface->rdata(offset, count, buffer);

}

static unsigned int interfacenode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct console_interfacenode *node = (struct console_interfacenode *)self->parent;

    return node->interface->wdata(offset, count, buffer);

}

void console_registerinterface(struct console_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.ctrl);
    system_addchild(&interface->node.base, &interface->node.data);

}

void console_unregisterinterface(struct console_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.ctrl);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void console_initinterface(struct console_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct console_interface));
    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");
    ctrl_init_consolesettings(&interface->settings);

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->node.interface = interface;
    interface->node.ctrl.read = interfacenode_ctrlread;
    interface->node.ctrl.write = interfacenode_ctrlwrite;
    interface->node.data.read = interfacenode_dataread;
    interface->node.data.write = interfacenode_datawrite;

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

