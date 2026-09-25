#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "service.h"

static void setname(struct service *service, char *name)
{

    buffer_clear(service->name, 32);

    if (name)
    {

        buffer_copy(service->name, name, cstring_length_zero(name));

        service->namehash = djb_hash(cstring_length(name), name);

    }

    else
    {

        service->namehash = 0;

    }

}

void service_register(struct service *service, unsigned int inode, char *name)
{

    setname(service, name);
    resource_register(&service->resource);

    service->inode = inode;

}

void service_unregister(struct service *service)
{

    resource_unregister(&service->resource);

    service->inode = 0;

}

void service_init(struct service *service)
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);
    list_init(&service->links);

}

