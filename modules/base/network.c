#include <fudge/module.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "network.h"

static struct network_node
{

    struct system_group base;
    struct base_network_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream mac;

} node[8];

static struct system_group root;
static struct system_group dev;

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_node *node = (struct network_node *)self->parent;
 
    if (offset)
        return 0;

    return node->interface->receive(node->device, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_node *node = (struct network_node *)self->parent;
 
    if (offset)
        return 0;

    return node->interface->send(node->device, count, buffer);

}

static unsigned int mac_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_node *node = (struct network_node *)self->parent;
    char mac[17];
    unsigned int i;

    memory_clear(mac, 17);

    for (i = 0; i < 6; i++)
        memory_write_paddednumber(mac, 17, node->interface->mac[i], 16, 2, i * 3);

    for (i = 2; i < 17; i += 3)
        mac[i] = ':';

    return memory_read(buffer, count, mac, 17, offset); 

}

static unsigned int find_node()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!node[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_node(struct network_node *node, struct base_network_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct network_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->mac, "mac");

    node->interface = interface;
    node->device = device;
    node->data.node.read = data_read;
    node->data.node.write = data_write;
    node->mac.node.read = mac_read;

}

void base_network_register_interface(struct base_network_interface *interface, struct base_device *device)
{

    unsigned int index = find_node();

    if (!index)
        return;

    init_node(&node[index], interface, device);
    system_group_add(&dev, &node[index].base.node);
    system_group_add(&node[index].base, &node[index].data.node);
    system_group_add(&node[index].base, &node[index].mac.node);

}

void base_network_register_protocol(unsigned short index, struct base_network_protocol *protocol)
{

}

void base_network_init_interface(struct base_network_interface *interface, unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer), unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_network_interface));

    interface->receive = receive;
    interface->send = send;

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

    memory_clear(node, sizeof (struct network_node) * 8);
    system_init_group(&root, "network");
    system_group_add(&root, &dev.node);
    system_init_group(&dev, "dev");
    system_register_node(&root.node);

}

