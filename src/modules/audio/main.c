#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "audio.h"

static unsigned int interfacedata_read(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    struct audio_interface *interface = self->resource->data;

    return interface->rdata(state->offset, count, buffer);

}

static unsigned int interfacedata_write(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    struct audio_interface *interface = self->resource->data;

    return interface->wdata(state->offset, count, buffer);

}

void audio_registerinterface(struct audio_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_registernode(&interface->root);

    interface->id = id;

}

void audio_unregisterinterface(struct audio_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_unregisternode(&interface->root);

}

void audio_initinterface(struct audio_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, RESOURCE_AUDIOINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "audio");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->data.resource = &interface->resource;
    interface->data.read = interfacedata_read;
    interface->data.write = interfacedata_write;

}

