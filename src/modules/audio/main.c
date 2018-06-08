#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "audio.h"

static struct system_node root;

void audio_registerinterface(struct audio_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&root, &interface->root);

}

void audio_unregisterinterface(struct audio_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&root, &interface->root);

}

void audio_initinterface(struct audio_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_AUDIOINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");

    interface->id = id;

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

