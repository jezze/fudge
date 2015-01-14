#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "mouse.h"

static struct system_node root;

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct mouse_interfacenode *node = (struct mouse_interfacenode *)self->parent;

    return node->interface->rdata(offset, count, buffer);

}

void mouse_registerinterface(struct mouse_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.data);

}

void mouse_unregisterinterface(struct mouse_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void mouse_initinterface(struct mouse_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct mouse_interface));
    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->rdata = rdata;
    interface->node.interface = interface;
    interface->node.data.read = interfacenode_dataread;

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

