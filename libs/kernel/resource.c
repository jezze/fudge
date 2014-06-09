#include <kernel.h>
#include "resource.h"

static struct list resources;

struct resource *resource_find(struct resource *resource)
{

    struct list_item *current = (resource) ? resource->item.next : resources.head;

    if (!current)
        return 0;

    return current->data;

}

void resource_register(struct resource *resource)
{

    list_add(&resources, &resource->item);

}

void resource_unregister(struct resource *resource)
{

    list_remove(&resources, &resource->item);

}

void resource_init(struct resource *resource, unsigned int type, void *data)
{

    memory_clear(resource, sizeof (struct resource));
    list_init_item(&resource->item, resource);

    resource->type = type;
    resource->data = data;

}

void resource_setup()
{

    list_init(&resources);

}

