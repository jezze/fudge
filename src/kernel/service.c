#include <fudge.h>
#include "resource.h"
#include "service.h"

void service_register(struct service *service)
{

    resource_register(&service->resource);

}

void service_init(struct service *service, char *name, struct resource *(*foreach)(struct resource *current), unsigned int (*getinode)(struct resource *current, unsigned int index))
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);

    service->name = name;
    service->foreach = foreach;
    service->getinode = getinode;

}

