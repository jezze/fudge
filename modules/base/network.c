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

static struct network_session
{

    struct system_group base;
    struct network_node *node;
    struct system_group device;
    struct system_stream control;
    char name[8];

} session[8];

static struct system_group root;
static struct system_group dev;
static struct system_stream clone;

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_session *session = (struct network_session *)self->parent;

    return memory_read(buffer, count, session->name, string_length(session->name), offset);

}

static unsigned int device_open(struct system_node *self)
{

    struct network_session *session = (struct network_session *)self->parent;

    return session->node->base.node.open(&session->node->base.node);

}

static unsigned int find_session()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!session[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_session(struct network_session *session, unsigned int id, struct network_node *node)
{

    memory_clear(session, sizeof (struct network_session));
    memory_write_number(session->name, 8, id, 10, 0);
    system_init_group(&session->base, session->name);
    system_init_group(&session->device, "device");
    system_init_stream(&session->control, "control");

    session->node = node;
    session->device.node.open = device_open;
    session->control.node.read = control_read;

}

static unsigned int clone_open(struct system_node *self)
{

    unsigned int index = find_session();

    if (!index)
        return 0;

    init_session(&session[index], index, &node[1]);
    system_group_add(&root, &session[index].base.node);
    system_group_add(&session[index].base, &session[index].device.node);
    system_group_add(&session[index].base, &session[index].control.node);

    return (unsigned int)&session[index].control;

}

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
    char *mac = "00:00:00:00:00:00";
    unsigned int i;

    for (i = 0; i < 6; i++)
        memory_write_paddednumber(mac, 17, node->interface->mac[i], 16, 2, i * 3);

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
    memory_clear(session, sizeof (struct network_session) * 8);
    system_init_group(&root, "network");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

