#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include "ipv6.h"

static struct ethernet_protocol ethernetprotocol;

static void ethernetprotocol_notify(struct ethernet_header *ethernetheader, void *buffer, unsigned int count)
{

    union event_message message;
    struct ipv6_header *header = buffer;
    unsigned int length = (header->length[0] << 8) | header->length[1];
    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_IPV6PROTOCOL)))
    {

        struct ipv6_protocol *protocol = current->data;

        if (protocol->id == header->next)
            protocol->notify(header, header + 1, length * 8);

    }

    event_create(&message, EVENT_DATA, EVENT_BROADCAST, 0);
    event_append(&message, count, buffer);
    kernel_multicast(&ethernetprotocol.data.states, &message);

}

void ipv6_registerprotocol(struct ipv6_protocol *protocol)
{

    resource_register(&protocol->resource);
    system_addchild(&protocol->root, &protocol->data);
    system_addchild(&ethernetprotocol.root, &protocol->root);

}

void ipv6_unregisterprotocol(struct ipv6_protocol *protocol)
{

    resource_unregister(&protocol->resource);
    system_removechild(&protocol->root, &protocol->data);
    system_removechild(&ethernetprotocol.root, &protocol->root);

}

void ipv6_initprotocol(struct ipv6_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ipv6_header *ipv6header, void *buffer, unsigned int count))
{

    resource_init(&protocol->resource, RESOURCE_IPV4PROTOCOL, protocol);
    system_initnode(&protocol->root, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&protocol->data, SYSTEM_NODETYPE_NORMAL, "data");

    protocol->id = id;
    protocol->notify = notify;

}

void module_init(void)
{

    ethernet_initprotocol(&ethernetprotocol, "ipv6", 0x86DD, ethernetprotocol_notify);

}

void module_register(void)
{

    ethernet_registerprotocol(&ethernetprotocol);

}

void module_unregister(void)
{

    ethernet_unregisterprotocol(&ethernetprotocol);

}

