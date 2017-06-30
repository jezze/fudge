#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "service.h"
#include "container.h"

struct container_mount *container_getfreemount(struct container *container)
{

    return (container->freemounts.tail) ? container->freemounts.tail->data : 0;

}

void container_usemount(struct container *container, struct container_mount *mount)
{

    list_move(&container->usedmounts, &mount->item);

}

void container_unusemount(struct container *container, struct container_mount *mount)
{

    list_move(&container->freemounts, &mount->item);

}

void container_init(struct container *container, unsigned int id)
{

    unsigned int i;

    resource_init(&container->resource, RESOURCE_CONTAINER, container);

    for (i = 0; i < CONTAINER_MOUNTS; i++)
    {

        resource_init(&container->mounts[i].resource, RESOURCE_CONTAINERMOUNT, &container->mounts[i]);
        list_inititem(&container->mounts[i].item, &container->mounts[i]);
        list_add(&container->freemounts, &container->mounts[i].item);

    }

    container->id = id;

}

