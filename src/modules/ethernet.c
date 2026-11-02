#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include "ethernet.h"

static struct node_operands operands;
static struct service service;

static unsigned int ondata(struct ethernet_interface *interface, unsigned int source, void *data, unsigned int count)
{

    if (interface->send)
    {

        interface->send(data, count);

        return MESSAGE_OK;

    }

    return MESSAGE_FAILED;

}

static unsigned int oninfo(struct ethernet_interface *interface, unsigned int source)
{

    if (interface->getmac)
    {

        struct event_ethernetinfo ethernetinfo;

        interface->getmac(ethernetinfo.address);

        return kernel_place(interface->inode, source, EVENT_ETHERNETINFO, sizeof (struct event_ethernetinfo), &ethernetinfo);

    }

    return MESSAGE_FAILED;

}

static unsigned int operands_place(unsigned int target, unsigned int source, unsigned int event, unsigned int count, void *data)
{

    struct resource *resource = pool_getnoderesource(target);

    if (resource)
    {

        struct ethernet_interface *interface = resource->data;

        switch (event)
        {

        case EVENT_LINK:
            return kernel_linknode(&interface->links, source);

        case EVENT_UNLINK:
            return kernel_unlinknode(&interface->links, source);

        case EVENT_DATA:
            return ondata(interface, source, data, count);

        case EVENT_INFO:
            return oninfo(interface, source);

        }

    }

    return MESSAGE_FAILED;

}

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(&interface->links, interface->inode, EVENT_DATA, count, buffer);

}

void ethernet_registerinterface(struct ethernet_interface *interface)
{

    interface->inode = pool_picknode(&interface->resource, &operands);

    resource_register(&interface->resource);
    service_addchild(&service, interface->inode);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);
    service_removechild(&service, interface->inode);
    pool_unpicknode(interface->inode);

    interface->inode = 0;

}

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, void (*getmac)(unsigned char address[6]), unsigned int (*send)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);
    list_init(&interface->links);

    interface->id = id;
    interface->getmac = getmac;
    interface->send = send;

}

void module_init(void)
{

    node_operands_init(&operands, 0, operands_place, 0);

    service_init(&service);

}

void module_register(void)
{

    service_registerdirectory(&service, "ethernet");

}

void module_unregister(void)
{

    service_unregisterdirectory(&service);

}

