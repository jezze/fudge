#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "service.h"

void service_register(struct service *service)
{

    resource_register(&service->resource);

}

void service_init(struct service *service, char *name, struct resource *(*foreach)(struct resource *current), unsigned int (*getinode)(struct resource *current, unsigned int index), unsigned int (*pick)(unsigned int source, struct message *message, unsigned int count, void *data), unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);

    service->name = name;
    service->namehash = (name) ? djb_hash(cstring_length(name), name) : 0;
    service->foreach = foreach;
    service->getinode = getinode;
    service->pick = pick;
    service->place = place;

}

