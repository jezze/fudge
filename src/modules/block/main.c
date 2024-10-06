#include <fudge.h>
#include <kernel.h>
#include "block.h"

static struct service service;

static struct node *service_match(unsigned int count, char *name)
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
                return &interface->node;

        }

    }

    return 0;

}

static unsigned int onblockrequest(struct block_interface *interface, unsigned int source, unsigned int count, void *data)
{

    struct event_blockrequest *blockrequest = data;

    return interface->onblockrequest(blockrequest->count, blockrequest->sector);

}

static unsigned int place(struct node *source, struct node *target, unsigned int event, unsigned int count, void *data)
{

    switch (event)
    {

    case EVENT_BLOCKREQUEST:
        return onblockrequest(target->interface, source->ichannel, count, data);

    }

    return MESSAGE_UNIMPLEMENTED;

}

void block_notifyblockresponse(struct block_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(&interface->node, EVENT_BLOCKRESPONSE, count, buffer);

}

void block_registerinterface(struct block_interface *interface)
{

    resource_register(&interface->resource);
    kernel_announce(&interface->node);

}

void block_unregisterinterface(struct block_interface *interface)
{

    resource_unregister(&interface->resource);
    kernel_unannounce(&interface->node);

}

void block_initinterface(struct block_interface *interface, unsigned int id, unsigned int (*onblockrequest)(unsigned int count, unsigned int sector))
{

    resource_init(&interface->resource, RESOURCE_BLOCKINTERFACE, interface);
    node_init(&interface->node, interface, place);

    interface->id = id;
    interface->onblockrequest = onblockrequest;

}

void module_init(void)
{

    service_init(&service, "block", service_match);
    service_register(&service);

}

