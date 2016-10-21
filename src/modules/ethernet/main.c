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

void ethernet_notify(struct ethernet_interface *interface, void *buffer, unsigned int count)
{

    struct ethernet_header *header = buffer;
    unsigned short type = (header->type[0] << 8) | header->type[1];
    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_ETHERNETPROTOCOL)))
    {

        struct ethernet_protocol *protocol = current->data;

        if (protocol->type == type)
            protocol->notify(interface, header + 1, count - 18);

    }

    system_multicast(&interface->datalinks, buffer, count);

}

static unsigned int interfaceaddr_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    struct ethernet_interface *interface = self->resource->data;

    return memory_read(buffer, count, interface->haddress, ETHERNET_ADDRSIZE, state->offset);

}

static unsigned int interfacedata_open(struct system_node *self, struct service_state *state)
{

    struct ethernet_interface *interface = self->resource->data;

    list_add(&interface->datalinks, &state->link);

    return state->id;

}

static unsigned int interfacedata_close(struct system_node *self, struct service_state *state)
{

    struct ethernet_interface *interface = self->resource->data;

    list_remove(&interface->datalinks, &state->link);

    return state->id;

}

static unsigned int interfacedata_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return system_readlink(&state->link, buffer, count);

}

void ethernet_registerinterface(struct ethernet_interface *interface, unsigned int id)
{

    resource_register(&interface->resource);
    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&interface->root, &interface->addr);
    system_addchild(&root, &interface->root);

    interface->id = id;

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

void ethernet_initinterface(struct ethernet_interface *interface, unsigned int (*send)(void *buffer, unsigned int count))
{

    resource_init(&interface->resource, RESOURCE_ETHERNETINTERFACE, interface);
    system_initresourcenode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "if", &interface->resource);
    system_initresourcenode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl", &interface->resource);
    system_initresourcenode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data", &interface->resource);
    system_initresourcenode(&interface->addr, SYSTEM_NODETYPE_NORMAL, "addr", &interface->resource);

    interface->send = send;
    interface->addr.read = interfaceaddr_read;
    interface->data.open = interfacedata_open;
    interface->data.close = interfacedata_close;
    interface->data.read = interfacedata_read;

}

void ethernet_initprotocol(struct ethernet_protocol *protocol, char *name, unsigned short type, void (*notify)(struct ethernet_interface *interface, void *buffer, unsigned int count))
{

    resource_init(&protocol->resource, RESOURCE_ETHERNETPROTOCOL, protocol);
    system_initresourcenode(&protocol->root, SYSTEM_NODETYPE_GROUP, name, &protocol->resource);
    system_initresourcenode(&protocol->data, SYSTEM_NODETYPE_MAILBOX, "data", &protocol->resource);

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

