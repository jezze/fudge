#include <fudge.h>
#include <kernel.h>
#include <net/ethernet.h>
#include <modules/system/system.h>
#include "ethernet.h"

static struct system_node root;

unsigned int ethernet_writeheader(struct ethernet_protocol *protocol, unsigned char *sha, unsigned char *tha, void *buffer)
{

    struct ethernet_header *header = buffer;

    header->type[0] = protocol->type >> 8;
    header->type[1] = protocol->type;

    memory_copy(header->sha, sha, ETHERNET_ADDRSIZE);
    memory_copy(header->tha, tha, ETHERNET_ADDRSIZE);

    return sizeof (struct ethernet_header);

}

void ethernet_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    struct ethernet_header *header = buffer;
    unsigned short type = (header->type[0] << 8) | header->type[1];
    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_ETHERNETPROTOCOL)))
    {

        struct ethernet_protocol *protocol = current->data;

        if (protocol->type == type)
            protocol->notify(interface, count - 18, header + 1);

    }

    system_multicast(&interface->data.links, count, buffer);

}

static void notifyaddinterface(struct ethernet_interface *interface)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_ETHERNETPROTOCOL)))
    {

        struct ethernet_protocol *protocol = current->data;

        protocol->addinterface(interface);

    }

}

static void notifyremoveinterface(struct ethernet_interface *interface)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_ETHERNETPROTOCOL)))
    {

        struct ethernet_protocol *protocol = current->data;

        protocol->removeinterface(interface);

    }

}

void ethernet_registerinterface(struct ethernet_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->info);
    system_addchild(&root, &interface->root);

    interface->id = id;

    notifyaddinterface(interface);

}

void ethernet_registerprotocol(struct ethernet_protocol *protocol)
{

    resource_register(&protocol->resource);
    system_addchild(&protocol->root, &protocol->data);
    system_addchild(&root, &protocol->root);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    resource_unregister(&interface->resource);
    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&interface->root, &interface->info);
    system_removechild(&root, &interface->root);
    notifyremoveinterface(interface);

}

void ethernet_unregisterprotocol(struct ethernet_protocol *protocol)
{

    resource_unregister(&protocol->resource);
    system_removechild(&protocol->root, &protocol->data);
    system_removechild(&root, &protocol->root);

}

void ethernet_initinterface(struct ethernet_interface *interface, char *name, unsigned int (*send)(unsigned int count, void *buffer))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, name, &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data", &interface->resource);
    system_initresourcenode(&interface->info, SYSTEM_NODETYPE_MAILBOX, "info", &interface->resource);

    interface->send = send;

}

void ethernet_initprotocol(struct ethernet_protocol *protocol, char *name, unsigned short type, void (*addinterface)(struct ethernet_interface *interface), void (*removeinterface)(struct ethernet_interface *interface), void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer))
{

    resource_init(&protocol->resource, RESOURCE_ETHERNETPROTOCOL, protocol);
    system_initresourcenode(&protocol->root, SYSTEM_NODETYPE_GROUP, name, &protocol->resource);
    system_initresourcenode(&protocol->data, SYSTEM_NODETYPE_MAILBOX, "data", &protocol->resource);

    protocol->type = type;
    protocol->addinterface = addinterface;
    protocol->removeinterface = removeinterface;
    protocol->notify = notify;

}

void module_init(void)
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "ethernet");

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

