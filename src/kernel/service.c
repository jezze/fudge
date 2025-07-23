#include <fudge.h>
#include "resource.h"
#include "service.h"

void service_register(struct service *service)
{

    resource_register(&service->resource);

}

void service_init(struct service *service, unsigned int (*pick)(unsigned int source, struct message *message, unsigned int count, void *data), unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);

    service->pick = pick;
    service->place = place;

}

