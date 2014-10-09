#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "network.h"

static struct system_node root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_interfacenode *interfacenode = (struct base_network_interfacenode *)self->parent;
 
    return interfacenode->interface->receive(count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_interfacenode *interfacenode = (struct base_network_interfacenode *)self->parent;
 
    return interfacenode->interface->send(count, buffer);

}

static unsigned int mac_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_interfacenode *interfacenode = (struct base_network_interfacenode *)self->parent;
    char *mac = "00:00:00:00:00:00";
    unsigned int i;

    for (i = 0; i < 6; i++)
        ascii_wzerovalue(mac, 17, interfacenode->interface->mac[i], 16, 2, i * 3);

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

void base_network_registerinterfacenode(struct base_network_interfacenode *interfacenode)
{

    system_addchild(&root, &interfacenode->base);
    system_addchild(&interfacenode->base, &interfacenode->data);
    system_addchild(&interfacenode->base, &interfacenode->mac);

}

void base_network_registerprotocol(struct base_network_protocol *protocol)
{

    resource_register(&protocol->resource);

}

void base_network_registerprotocolnode(struct base_network_protocolnode *protocolnode)
{

    system_addchild(&root, &protocolnode->base);

}

void base_network_unregisterinterface(struct base_network_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void base_network_unregisterinterfacenode(struct base_network_interfacenode *interfacenode)
{

}

void base_network_unregisterprotocol(struct base_network_protocol *protocol)
{

    resource_unregister(&protocol->resource);

}

void base_network_unregisterprotocolnode(struct base_network_protocolnode *protocolnode)
{

}

void base_network_initinterface(struct base_network_interface *interface, unsigned int (*receive)(unsigned int count, void *buffer), unsigned int (*send)(unsigned int count, void *buffer), void *(*getpacket)(), void (*dumppacket)())
{

    memory_clear(interface, sizeof (struct base_network_interface));
    base_initinterface(&interface->base, BASE_INTERFACE_TYPE_NETWORK);

    interface->receive = receive;
    interface->send = send;
    interface->getpacket = getpacket;
    interface->dumppacket = dumppacket;

}

void base_network_initinterfacenode(struct base_network_interfacenode *interfacenode, struct base_device *device, struct base_network_interface *interface)
{

    memory_clear(interfacenode, sizeof (struct base_network_interfacenode));
    system_initnode(&interfacenode->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, device->bus->name);
    system_initnode(&interfacenode->data, SYSTEM_NODETYPE_NORMAL, "data");
    system_initnode(&interfacenode->mac, SYSTEM_NODETYPE_NORMAL, "mac");

    interfacenode->interface = interface;
    interfacenode->data.read = data_read;
    interfacenode->data.write = data_write;
    interfacenode->mac.read = mac_read;

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

void base_network_initprotocolnode(struct base_network_protocolnode *protocolnode, struct base_network_protocol *protocol)
{

    memory_clear(protocolnode, sizeof (struct base_network_protocolnode));
    system_initnode(&protocolnode->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, protocol->name);

    protocolnode->protocol = protocol;

}

void base_network_setup()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "network");
    system_registernode(&root);

}

