#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include "sound.h"

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct sound_interface *interface = self->resource->data;

    return interface->rdata(offset, size, count, buffer);

}

static unsigned int interfacenode_datawrite(struct system_node *self, unsigned int offset, unsigned int size, unsigned int count, void *buffer)
{

    struct sound_interface *interface = self->resource->data;

    return interface->wdata(offset, size, count, buffer);

}

void sound_registerinterface(struct sound_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void sound_unregisterinterface(struct sound_interface *interface)
{

    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void sound_initinterface(struct sound_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int size, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, 0, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "sound");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->data.resource = &interface->resource;
    interface->data.read = interfacenode_dataread;
    interface->data.write = interfacenode_datawrite;

}

