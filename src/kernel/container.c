#include <fudge.h>
#include "resource.h"
#include "binary.h"
#include "container.h"
#include "task.h"
#include "service.h"

void container_initdescriptor(struct task_descriptor *descriptor, struct task *task)
{

    list_inititem(&descriptor->link, descriptor);

    descriptor->task = task;
    descriptor->server = 0;
    descriptor->id = 0;
    descriptor->offset = 0;
    descriptor->current = 0;

}

void container_init(struct container *container)
{

    unsigned int i;

    resource_init(&container->resource, RESOURCE_CONTAINER, container);

    for (i = 0; i < CONTAINER_MOUNTS; i++)
        resource_init(&container->mounts[i].resource, RESOURCE_CONTAINERMOUNT, &container->mounts[i]);

}

