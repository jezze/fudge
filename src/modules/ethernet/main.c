#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include "ethernet.h"

static struct node_operands operands;

static unsigned int ondata(struct ethernet_interface *interface, unsigned int source, void *data, unsigned int count)
{

    if (interface->ondata)
    {

        interface->ondata(data, count);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int oninfo(struct ethernet_interface *interface, unsigned int source)
{

    if (interface->oninfo)
    {

        struct event_ethernetinfo ethernetinfo;

        interface->oninfo(&ethernetinfo);

        return kernel_place(interface->inode, source, EVENT_ETHERNETINFO, sizeof (struct event_ethernetinfo), &ethernetinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(struct resource *resource, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct ethernet_interface *interface = resource->data;

    switch (event)
    {

    case EVENT_LINK:
        return kernel_linknode(&interface->service, source);

    case EVENT_UNLINK:
        return kernel_unlinknode(&interface->service, source);

    case EVENT_DATA:
        return ondata(interface, source, data, count);

    case EVENT_INFO:
        return oninfo(interface, source);

    }

    return MESSAGE_FAILED;

}

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(&interface->service, interface->inode, EVENT_DATA, count, buffer);

}

void ethernet_registerinterface(struct ethernet_interface *interface)
{

    resource_register(&interface->resource);
    service_register(&interface->service, interface->inode);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);
    service_unregister(&interface->service);

}

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, void (*oninfo)(struct event_ethernetinfo *ethernetinfo), unsigned int (*ondata)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);
    service_init(&interface->service, "ethernet");

    interface->id = id;
    interface->inode = pool_picknode();
    interface->oninfo = oninfo;
    interface->ondata = ondata;

    if (interface->inode)
    {

        struct node *node = pool_getnode(interface->inode);

        node_reset(node, &interface->resource, &operands);

    }

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place);

}

