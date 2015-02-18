#include <fudge.h>
#include "resource.h"
#include "vfs.h"
#include "container.h"

void container_init(struct container *container)
{

    unsigned int i;

    resource_init(&container->resource, RESOURCE_CONTAINER, container);

    for (i = 0; i < CONTAINER_CHANNELS; i++)
        vfs_initchannel(&container->channels[i]);

    for (i = 0; i < CONTAINER_MOUNTS; i++)
        vfs_initmount(&container->mounts[i]);

}

