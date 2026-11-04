#include <fudge.h>
#include "resource.h"
#include "service.h"

void service_register(struct service *service, unsigned int inode)
{

    if (inode)
    {

        resource_register(&service->resource);

        service->inode = inode;

    }

}

void service_unregister(struct service *service)
{

    if (service->inode)
    {

        resource_unregister(&service->resource);

        service->inode = 0;

    }

}

void service_init(struct service *service, char *name)
{

    resource_init(&service->resource, RESOURCE_SERVICE, service, 0);
    buffer_clear(service->name, SERVICE_NAMESIZE);
    buffer_copy(service->name, name, cstring_length_zero(name));

}

