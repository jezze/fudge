#include <fudge/module.h>
#include <system/system.h>
#include "base.h"
#include "network.h"

struct network_group
{

    struct system_group base;
    struct base_network_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream mac;

};

static struct system_group root;
static struct system_group dev;
static struct network_group groups[8];

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_group *group = (struct network_group *)self->parent;
 
    if (offset)
        return 0;

    return group->interface->receive(group->device, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_group *group = (struct network_group *)self->parent;
 
    if (offset)
        return 0;

    return group->interface->send(group->device, count, buffer);

}

static unsigned int mac_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_group *group = (struct network_group *)self->parent;
    char *mac = "00:00:00:00:00:00";
    unsigned int i;

    for (i = 0; i < 6; i++)
        memory_write_paddednumber(mac, 17, group->interface->mac[i], 16, 2, i * 3);

    return memory_read(buffer, count, mac, 17, offset); 

}

static unsigned int find_group()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!groups[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_group(struct network_group *group, struct base_network_interface *interface, struct base_device *device)
{

    memory_clear(group, sizeof (struct network_group));
    system_init_group(&group->base, device->module.name);
    system_init_stream(&group->data, "data");
    system_init_stream(&group->mac, "mac");

    group->interface = interface;
    group->device = device;

    group->data.node.read = data_read;
    group->data.node.write = data_write;
    group->mac.node.read = mac_read;

}

void base_network_register_interface(struct base_network_interface *interface, struct base_device *device)
{

    unsigned int index = find_group();

    if (!index)
        return;

    init_group(&groups[index], interface, device);
    system_group_add(&groups[index].base, &groups[index].data.node);
    system_group_add(&groups[index].base, &groups[index].mac.node);
    system_group_add(&dev, &groups[index].base.node);

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

    system_init_group(&root, "network");
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_register_node(&root.node);

}

