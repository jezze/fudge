#include <fudge.h>
#include <kernel.h>
#include "audio.h"

void audio_registerinterface(struct audio_interface *interface)
{

    resource_register(&interface->resource);

}

void audio_unregisterinterface(struct audio_interface *interface)
{

    resource_unregister(&interface->resource);

}

void audio_initinterface(struct audio_interface *interface, unsigned int id)
{

    resource_init(&interface->resource, RESOURCE_AUDIOINTERFACE, interface);
    node_init(&interface->node, 0, interface, 0);

    interface->id = id;

}

