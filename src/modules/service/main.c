#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

static struct system_node service;
static struct system_node nodes[64];
static unsigned int numnodes;
static char namebuffer[1024];
static unsigned int nameoffset;

static unsigned int notify(struct list *links, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct list_item *current;

    spinlock_acquire(&links->spinlock);

    for (current = links->head; current; current = current->next)
    {

        struct link *link = current->data;

        if (link->target != source)
            kernel_place(source, link->target, event, count, data);

    }

    spinlock_release(&links->spinlock);

    return count;

}

static unsigned int service_create(void *name, unsigned int length)
{

    struct system_node *node = &nodes[numnodes];

    numnodes++;

    system_initnode(node, SYSTEM_NODETYPE_NORMAL, namebuffer + nameoffset);

    node->operations.notify = notify;

    nameoffset += buffer_write(namebuffer, 1024, name, length, nameoffset);
    nameoffset += cstring_write_zero(namebuffer, 1024, nameoffset);

    system_addchild(&service, node);

    return (unsigned int)node;

}

static unsigned int service_destroy(void)
{

    return 0;

}

void module_init(void)
{

    system_initnode(&service, SYSTEM_NODETYPE_GROUP, "service");

    service.operations.create = service_create;
    service.operations.destroy = service_destroy;

}

void module_register(void)
{

    system_registernode(&service);

}

void module_unregister(void)
{

    system_unregisternode(&service);

}

