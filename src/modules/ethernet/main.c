#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include "ethernet.h"

static struct system_node root;

static struct ethernet_interface *findinterface(void *address)
{

    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_ETHERNETINTERFACE)))
    {

        struct ethernet_interface *interface = current->data;

        if (interface->matchaddress(address, ETHERNET_ADDRSIZE))
            return interface;

    }

    return 0;

}

void *ethernet_writehead(void *buffer, unsigned int type, unsigned char *sha, unsigned char *tha)
{

    struct ethernet_header *header = buffer;

    header->type[0] = type >> 8;
    header->type[1] = type;

    memory_copy(header->sha, sha, ETHERNET_ADDRSIZE);
    memory_copy(header->tha, tha, ETHERNET_ADDRSIZE);

    return header + 1;

}

void ethernet_send(void *buffer, unsigned int count)
{

    struct ethernet_header *header = buffer;
    struct ethernet_interface *interface = findinterface(header->sha);

    if (!interface)
        return;

    interface->send(buffer, count);

}

void ethernet_notify(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    union event_message message;
    struct ethernet_header *header = buffer;
    unsigned int type = (header->type[0] << 8) | header->type[1];
    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_ETHERNETPROTOCOL)))
    {

        struct ethernet_protocol *protocol = current->data;

        if (protocol->type == type)
            protocol->notify(header, header + 1, count - 18);

    }

    event_create(&message, EVENT_DATA, EVENT_BROADCAST, 0);
    event_adddata(&message);
    event_appenddata(&message, count, buffer);
    kernel_multicast(&interface->data.states, &message);

}

void ethernet_registerinterface(struct ethernet_interface *interface)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->addr);
    system_addchild(&root, &interface->root);

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
    system_removechild(&interface->root, &interface->addr);
    system_removechild(&root, &interface->root);

}

void ethernet_unregisterprotocol(struct ethernet_protocol *protocol)
{

    resource_unregister(&protocol->resource);
    system_removechild(&protocol->root, &protocol->data);
    system_removechild(&root, &protocol->root);

}

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int id, unsigned int (*matchaddress)(void *buffer, unsigned int count), unsigned int (*send)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);
    system_initnode(&interface->root, SYSTEM_NODETYPE_MULTIGROUP, "if");
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interface->addr, SYSTEM_NODETYPE_NORMAL, "addr");

    interface->id = id;
    interface->matchaddress = matchaddress;
    interface->send = send;

}

void ethernet_initprotocol(struct ethernet_protocol *protocol, char *name, unsigned int type, void (*notify)(struct ethernet_header *header, void *buffer, unsigned int count))
{

    resource_init(&protocol->resource, RESOURCE_ETHERNETPROTOCOL, protocol);
    system_initnode(&protocol->root, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&protocol->data, SYSTEM_NODETYPE_NORMAL, "data");

    protocol->type = type;
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

