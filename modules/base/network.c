#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "network.h"

static struct interface_node
{

    struct system_group base;
    struct system_stream data;
    struct system_stream mac;
    struct base_device device;

} inode[8];

static struct system_group root;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    struct base_network_interface *interface = (struct base_network_interface *)node->device.interface;
 
    if (offset)
        return 0;

    return interface->receive(node->device.bus, node->device.id, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    struct base_network_interface *interface = (struct base_network_interface *)node->device.interface;
 
    if (offset)
        return 0;

    return interface->send(node->device.bus, node->device.id, count, buffer);

}

static unsigned int mac_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    struct base_network_interface *interface = (struct base_network_interface *)node->device.interface;
    char *mac = "00:00:00:00:00:00";
    unsigned int i;

    for (i = 0; i < 6; i++)
        ascii_write_zerovalue(mac, 17, interface->mac[i], 16, 2, i * 3);

    return memory_read(buffer, count, mac, 17, offset); 

}

static unsigned int find_inode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!inode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_inode(struct interface_node *node, struct base_interface *interface, struct base_bus *bus, unsigned int id)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, bus->name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->mac, "mac");
    base_init_device(&node->device, interface, bus, id);

    node->data.node.read = data_read;
    node->data.node.write = data_write;
    node->mac.node.read = mac_read;

}

void base_network_connect_interface(struct base_interface *interface, struct base_bus *bus, unsigned int id)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, bus, id);
    system_group_add(&root, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].data.node);
    system_group_add(&inode[index].base, &inode[index].mac.node);

}

void base_network_register_interface(struct base_network_interface *interface)
{

    base_register_interface(&interface->base);

}

void base_network_register_protocol(struct base_network_protocol *protocol)
{

}

void base_network_unregister_interface(struct base_network_interface *interface)
{

    base_unregister_interface(&interface->base);

}

void base_network_unregister_protocol(struct base_network_protocol *protocol)
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

void base_network_setup()
{

    memory_clear(inode, sizeof (struct interface_node) * 8);
    system_init_group(&root, "network");
    system_register_node(&root.node);

}

