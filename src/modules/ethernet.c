#include <fudge.h>
#include <net.h>
#include <kernel.h>
#include "ethernet.h"

static struct unitsrv unitsrv;

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

        struct message_ethernetinfo ethernetinfo;

        interface->getmac(ethernetinfo.address);

        return kernel_place(interface->resource.inode, source, MESSAGE_TYPE_ETHERNETINFO, sizeof (struct message_ethernetinfo), &ethernetinfo);

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

        case MESSAGE_TYPE_LINK:
            return kernel_linknode(resource, source);

        case MESSAGE_TYPE_UNLINK:
            return kernel_unlinknode(resource, source);

        case MESSAGE_TYPE_DATA:
            return ondata(interface, source, data, count);

        case MESSAGE_TYPE_INFO:
            return oninfo(interface, source);

        }

    }

    return MESSAGE_FAILED;

}

void ethernet_notifydata(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    kernel_notify(&interface->resource, MESSAGE_TYPE_DATA, count, buffer);

}

void ethernet_registerinterface(struct ethernet_interface *interface)
{

    resource_register(&interface->resource);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);

}

void ethernet_initinterface(struct ethernet_interface *interface, void (*getmac)(unsigned char address[6]), unsigned int (*send)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface, operands_place);

    interface->getmac = getmac;
    interface->send = send;

}

void module_init(void)
{

    unitsrv_init(&unitsrv, RESOURCE_ETHERNETINTERFACE, "ethernet");

}

void module_register(void)
{

    unitsrv_register(&unitsrv);

}

void module_unregister(void)
{

    unitsrv_unregister(&unitsrv);

}

