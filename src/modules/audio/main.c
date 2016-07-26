#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "audio.h"

static struct system_node root;

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
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void audio_unregisterinterface(struct audio_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&root, &interface->root);

}

void audio_initinterface(struct audio_interface *interface, unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&interface->resource, RESOURCE_AUDIOINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data", &interface->resource);

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->data.read = interfacedata_read;
    interface->data.write = interfacedata_write;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "audio");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

