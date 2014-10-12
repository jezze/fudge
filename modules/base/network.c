#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "network.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_node *node = (struct base_network_node *)self->parent;
 
    return node->interface->receive(count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_node *node = (struct base_network_node *)self->parent;
 
    return node->interface->send(count, buffer);

}

static unsigned int mac_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_node *node = (struct base_network_node *)self->parent;
    char *mac = "00:00:00:00:00:00";
    unsigned int i;

    for (i = 0; i < 6; i++)
        ascii_wzerovalue(mac, 17, node->interface->mac[i], 16, 2, i * 3);

    return memory_read(buffer, count, mac, 17, offset); 

}

void base_network_notify(struct base_network_interface *interface)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, BASE_NETWORK_RESOURCE_PROTOCOL)))
    {

        struct base_network_protocol *protocol = current->data;

        if (protocol->match(interface))
            return;

    }

}

void base_network_registerinterface(struct base_network_interface *interface)
{

    base_registerinterface(&interface->base);

}

void base_network_registernode(struct base_network_node *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);
    system_addchild(&node->base, &node->mac);

}

void base_network_registerprotocol(struct base_network_protocol *protocol)
{

    resource_register(&protocol->resource);

}

void base_network_unregisterinterface(struct base_network_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_network_unregisternode(struct base_network_node *node)
{

}

void base_network_unregisterprotocol(struct base_network_protocol *protocol)
{

    resource_unregister(&protocol->resource);

}

void base_network_initinterface(struct base_network_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*receive)(unsigned int count, void *buffer), unsigned int (*send)(unsigned int count, void *buffer), void *(*getpacket)(), void (*dumppacket)())
{

    memory_clear(interface, sizeof (struct base_network_interface));
    base_initinterface(&interface->base, bus, id);

    interface->receive = receive;
    interface->send = send;
    interface->getpacket = getpacket;
    interface->dumppacket = dumppacket;

}

void base_network_initnode(struct base_network_node *node, struct base_network_interface *interface)
{

    memory_clear(node, sizeof (struct base_network_node));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&node->mac, SYSTEM_NODETYPE_NORMAL, "mac");

    node->interface = interface;
    node->data.read = data_read;
    node->data.write = data_write;
    node->mac.read = mac_read;

}

void base_network_initprotocol(struct base_network_protocol *protocol, char *name, unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*match)(struct base_network_interface *interface))
{

    memory_clear(protocol, sizeof (struct base_network_protocol));
    resource_init(&protocol->resource, BASE_NETWORK_RESOURCE_PROTOCOL, protocol);

    protocol->name = name;
    protocol->match = match;
    protocol->read = read;
    protocol->write = write;

}

void base_network_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "network");
    system_registernode(&root);

}

