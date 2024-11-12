#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct service service;

static unsigned int service_match(unsigned int count, char *name)
{

    if (count == 2 && buffer_match(name, ":", 1))
    {

        struct resource *current = 0;
        unsigned int index = cstring_toint(name[1]);
        unsigned int i;

        for (i = 0; (current = resource_foreachtype(current, RESOURCE_BLOCKINTERFACE)); i++)
        {

            struct block_interface *interface = current->data;

            if (i == index)
                return interface->inode;

        }

    }

    return 0;

}

static unsigned int onblockrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_blockrequest *blockrequest = data;

    return interface->onblockrequest(source, blockrequest->count, blockrequest->sector);

}

static unsigned int place(unsigned int source, unsigned int target, unsigned int event, unsigned int count, void *data)
{

    struct block_interface *interface = kernel_getinterface(target);

    switch (event)
    {

    case EVENT_BLOCKREQUEST:
        return onblockrequest(interface, source, count, data);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void block_notifyblockresponse(struct block_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(interface->inode, &interface->resource.targets, EVENT_BLOCKRESPONSE, count, buffer);

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);

}

void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int (*onblockrequest)(unsigned int source, unsigned int count, unsigned int sector))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);

    interface->id = id;
    interface->inode = kernel_link(&interface->resource.sources, 0, &interface->resource, place);
    interface->onblockrequest = onblockrequest;

}

void module_init(void)
{

    service_init(&service, "block", service_match);
    service_register(&service);

}

