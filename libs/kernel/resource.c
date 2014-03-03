#include <kernel.h>
#include "resource.h"

static struct list containers;
static struct list tasks;

void resource_register_container(struct resource *resource)
{

    list_add(&containers, &resource->item);

}

void resource_register_task(struct resource *resource)
{

    list_add(&tasks, &resource->item);

}

void resource_init(struct resource *resource, void *data)
{

    memory_clear(resource, sizeof (struct resource));
    list_init_item(&resource->item, data);

}

