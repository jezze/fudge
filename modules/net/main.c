#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include "net.h"

struct net_interface_group
{

    struct system_group base;
    struct net_interface *interface;
    struct system_stream data;
    struct system_stream mac;

};

static struct system_group root;
static struct net_interface_group interfaces[32];
static unsigned int ninterfaces;

unsigned int data_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_interface_group *group = (struct net_interface_group *)self->node.parent;

    if (offset)
        return 0;

    return group->interface->receive(group->interface, count, buffer);

}

unsigned int data_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_interface_group *group = (struct net_interface_group *)self->node.parent;

    if (offset)
        return 0;

    return group->interface->send(group->interface, count, buffer);

}

unsigned int mac_read(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct net_interface_group *group = (struct net_interface_group *)self->node.parent;
    char *mac = "00:00:00:00:00:00\n";
    unsigned int i;

    for (i = 0; i < 6; i++)
    {

        if (group->interface->mac[i] < 0x10)
            memory_write_number(mac, 18, group->interface->mac[i], 16, i * 3 + 1);
        else
            memory_write_number(mac, 18, group->interface->mac[i], 16, i * 3);

    }

    return memory_read(buffer, count, mac, 18, offset); 

}

unsigned int mac_write(struct system_stream *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void net_register_interface(struct net_interface *interface)
{

    struct net_interface_group *group = &interfaces[ninterfaces];

    group->interface = interface;

    system_init_group(&group->base, interface->driver->module.name);
    system_group_add(&root, &group->base.node);
    system_init_stream(&group->data, "data", data_read, data_write);
    system_group_add(&group->base, &group->data.node);
    system_init_stream(&group->mac, "mac", mac_read, mac_write);
    system_group_add(&group->base, &group->mac.node);

    ninterfaces++;

}

void net_register_protocol(unsigned short index, struct net_protocol *protocol)
{

}

void net_unregister_interface(struct net_interface *interface)
{

}

void net_unregister_protocol(unsigned short index)
{

}

void net_init_interface(struct net_interface *interface, struct base_driver *driver, unsigned int (*receive)(struct net_interface *self, unsigned int count, void *buffer), unsigned int (*send)(struct net_interface *self, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct net_interface));

    interface->driver = driver;
    interface->receive = receive;
    interface->send = send;

}

void net_init_protocol(struct net_protocol *protocol, char *name, unsigned int (*read)(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(protocol, sizeof (struct net_protocol));

    protocol->name = name;
    protocol->read = read;
    protocol->write = write;

}

void init()
{

    ninterfaces = 0;

    system_init_group(&root, "net");
    system_register_node(&root.node);

}

void destroy()
{

    system_unregister_node(&root.node);

}

