#include <fudge.h>
#include "resource.h"
#include "service.h"

void service_register(struct service *service)
{

    resource_register(&service->resource);

}

void service_init(struct service *service, char *name, unsigned int (*match)(unsigned int count, char *name))
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);

    service->name = name;
    service->match = match;

}

