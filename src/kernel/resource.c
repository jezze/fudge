#include <fudge.h>
#include "resource.h"
#include "pool.h"

static struct list resources;

static struct resource *nextresource(struct resource *resource)
{

    struct list_item *next = list_next_unsafe(&resources, &resource->item);

    return (next) ? next->data : 0;

}

struct resource *resource_foreachtype_unsafe(struct resource *resource, unsigned int type)
{

    while ((resource = nextresource(resource)))
    {

        if (resource->type == type)
            break;

    }

    return resource;

}

struct resource *resource_foreachtype(struct resource *resource, unsigned int type)
{

    spinlock_acquire(&resources.spinlock);

    resource = resource_foreachtype_unsafe(resource, type);

    spinlock_release(&resources.spinlock);

    return resource;

}

void resource_lock(void)
{

    spinlock_acquire(&resources.spinlock);

}

void resource_unlock(void)
{

    spinlock_release(&resources.spinlock);

}

void resource_register(struct resource *resource)
{

    list_add(&resources, &resource->item);

}

void resource_unregister(struct resource *resource)
{

    list_remove(&resources, &resource->item);
    pool_detachnode(resource);

}

void resource_operands_init(struct resource_operands *operands, unsigned int (*place)(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data), unsigned long (*locate)(unsigned int id))
{

    operands->place = place;
    operands->locate = locate;

}

void resource_init(struct resource *resource, unsigned int type, void *data, struct resource_operands *operands)
{

    list_inititem(&resource->item, resource);
    list_init(&resource->links);

    resource->type = type;
    resource->data = data;
    resource->inode = 0;
    resource->operands = operands;

}

void resource_setup(void)
{

    list_init(&resources);

}

