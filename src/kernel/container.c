#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "task.h"
#include "service.h"
#include "container.h"

struct service_server *container_getserver(struct container *container, unsigned int server)
{

    return &container->servers[server & (CONTAINER_SERVERS - 1)];

}

struct service_server *container_getfreeserver(struct container *container)
{

    return (container->freeservers.tail) ? container->freeservers.tail->data : 0;

}

void container_useserver(struct container *container, struct service_server *server)
{

    list_move(&container->usedservers, &server->item);

}

void container_unuseserver(struct container *container, struct service_server *server)
{

    list_move(&container->freeservers, &server->item);

}

struct container_mount *container_getmount(struct container *container, unsigned int mount)
{

    return &container->mounts[mount & (CONTAINER_MOUNTS - 1)];

}

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

    for (i = 0; i < CONTAINER_SERVERS; i++)
    {

        service_initserver(&container->servers[i]);
        list_add(&container->freeservers, &container->servers[i].item);

    }

    container->id = id;

}

