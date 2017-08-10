#include <fudge.h>
#include "resource.h"
#include "container.h"

void container_init(struct container *container, unsigned int id)
{

    resource_init(&container->resource, RESOURCE_CONTAINER, container);

    container->id = id;

}

