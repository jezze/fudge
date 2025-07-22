#include <fudge.h>
#include <hash.h>
#include "resource.h"
#include "service.h"

void service_register(struct service *service)
{

    resource_register(&service->resource);

}

void service_init(struct service *service, char *name, struct resource *(*foreach)(struct resource *current), unsigned int (*getinode)(struct resource *current, unsigned int index), unsigned int (*getinodename)(unsigned int namehash), unsigned int (*place)(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data))
{

    resource_init(&service->resource, RESOURCE_SERVICE, service);

    service->name = name;
    service->namehash = djb_hash(cstring_length(name), name);
    service->foreach = foreach;
    service->getinode = getinode;
    service->getinodename = getinodename;
    service->place = place;

}

