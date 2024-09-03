#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "audio.h"

void audio_registerinterface(struct audio_interface *interface)
{

    resource_register(&interface->resource);

}

void audio_unregisterinterface(struct audio_interface *interface)
{

    resource_unregister(&interface->resource);

}

void audio_initinterface(struct audio_interface *interface, unsigned int id, unsigned int ichannel)
{

    resource_init(&interface->resource, RESOURCE_AUDIOINTERFACE, interface);

    interface->id = id;
    interface->ichannel = ichannel;

}

