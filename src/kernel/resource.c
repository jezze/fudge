#include <fudge.h>
#include "resource.h"

static struct list resources;

static struct resource *nextresource(struct resource *resource)
{

    struct list_item *current = (resource) ? resource->item.next : resources.head;

    return (current) ? current->data : 0;

}

struct resource *resource_foreach(struct resource *resource)
{

    spinlock_acquire(&resources.spinlock);

    resource = nextresource(resource);

    spinlock_release(&resources.spinlock);

    return resource;

}

struct resource *resource_foreachtype(struct resource *resource, unsigned int type)
{

    spinlock_acquire(&resources.spinlock);

    while ((resource = nextresource(resource)))
    {

        if (resource->type == type)
            break;

    }

    spinlock_release(&resources.spinlock);

    return resource;

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

    list_inititem(&resource->item, resource);

    resource->type = type;
    resource->data = data;

}

void resource_setup(void)
{

    list_init(&resources);

}

