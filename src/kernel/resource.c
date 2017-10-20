#include <fudge.h>
#include "resource.h"

static struct list resources;
static struct spinlock resourcelock;

struct resource *resource_find(struct resource *resource)
{

    struct list_item *current;

    spinlock_acquire(&resourcelock);

    current = (resource) ? resource->item.next : resources.head;

    spinlock_release(&resourcelock);

    return (current) ? current->data : 0;

}

struct resource *resource_findtype(struct resource *resource, unsigned int type)
{

    struct resource *current = resource;

    while ((current = resource_find(current)))
    {

        if (current->type == type)
            return current;

    }

    return 0;

}

void resource_register(struct resource *resource)
{

    list_lockadd(&resources, &resource->item, &resourcelock);

}

void resource_unregister(struct resource *resource)
{

    list_lockremove(&resources, &resource->item, &resourcelock);

}

void resource_init(struct resource *resource, unsigned int type, void *data)
{

    list_inititem(&resource->item, resource);

    resource->type = type;
    resource->data = data;

}

