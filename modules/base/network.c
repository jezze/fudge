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
 
    if (offset)
        return 0;

    return node->interface->receive(node->device->bus, node->device->id, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_node *node = (struct base_network_node *)self->parent;
 
    if (offset)
        return 0;

    return node->interface->send(node->device->bus, node->device->id, count, buffer);

}

static unsigned int mac_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct base_network_node *node = (struct base_network_node *)self->parent;
    char *mac = "00:00:00:00:00:00";
    unsigned int i;

    for (i = 0; i < 6; i++)
        ascii_write_zerovalue(mac, 17, node->interface->mac[i], 16, 2, i * 3);

    return memory_read(buffer, count, mac, 17, offset); 

}

void base_network_register_interface(struct base_network_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_network_register_protocol(struct base_network_protocol *protocol)
{

}

void base_network_register_node(struct base_network_node *node)
{

    system_add_child(&root, &node->base);
    system_add_child(&node->base, &node->data);
    system_add_child(&node->base, &node->mac);

}

void base_network_unregister_interface(struct base_network_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_network_unregister_protocol(struct base_network_protocol *protocol)
{

}

void base_network_unregister_node(struct base_network_node *node)
{

}

void base_network_init_interface(struct base_network_interface *interface, unsigned int (*receive)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer), unsigned int (*send)(struct base_bus *bus, unsigned int id, unsigned int count, void *buffer), void *(*get_packet)(struct base_bus *bus, unsigned int id), void (*dump_packet)(struct base_bus *bus, unsigned int id))
{

    memory_clear(interface, sizeof (struct base_network_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_NETWORK);

    interface->receive = receive;
    interface->send = send;
    interface->get_packet = get_packet;
    interface->dump_packet = dump_packet;

}

void base_network_init_protocol(struct base_network_protocol *protocol, char *name, unsigned int (*read)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(protocol, sizeof (struct base_network_protocol));

    protocol->name = name;
    protocol->read = read;
    protocol->write = write;

}

void base_network_init_node(struct base_network_node *node, struct base_device *device, struct base_network_interface *interface)
{

    memory_clear(node, sizeof (struct base_network_node));
    system_init_multigroup(&node->base, device->bus->name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->mac, "mac");

    node->device = device;
    node->interface = interface;
    node->data.read = data_read;
    node->data.write = data_write;
    node->mac.read = mac_read;

}

void base_network_setup()
{

    system_init_group(&root, "network");
    system_register_node(&root);

}

