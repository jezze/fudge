#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "base.h"
#include "network.h"

static struct interface_node
{

    struct system_group base;
    struct base_network_interface *interface;
    struct base_device *device;
    struct system_stream data;
    struct system_stream mac;

} inode[8];

static struct protocol_node
{

    struct system_group base;
    struct base_network_protocol *protocol;

} pnode[8];

static struct session_node
{

    struct system_group base;
    struct interface_node *inode;
    struct protocol_node *pnode;
    struct system_group device;
    struct system_group protocol;
    struct system_stream control;
    char name[8];

} snode[8];

static struct system_group root;
static struct system_group dev;
static struct system_group protocols;
static struct system_stream clone;

static unsigned int control_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct session_node *node = (struct session_node *)self->parent;

    return memory_read(buffer, count, node->name, ascii_length(node->name), offset);

}

static unsigned int device_open(struct system_node *self)
{

    struct session_node *node = (struct session_node *)self->parent;

    return node->inode->base.node.open(&node->inode->base.node);

}

static unsigned int device_child(struct system_node *self, unsigned int count, const char *path)
{

    struct session_node *node = (struct session_node *)self->parent;

    return node->inode->base.node.child(&node->inode->base.node, count, path);

}

static unsigned int find_snode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!snode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_snode(struct session_node *node, unsigned int id, struct interface_node *inode, struct protocol_node *pnode)
{

    memory_clear(node, sizeof (struct session_node));
    ascii_fromint(node->name, 8, id, 10);
    system_init_group(&node->base, node->name);
    system_init_group(&node->device, "device");
    system_init_group(&node->protocol, "protocol");
    system_init_stream(&node->control, "control");

    node->inode = inode;
    node->pnode = pnode;
    node->device.node.open = device_open;
    node->device.node.child = device_child;
    node->control.node.read = control_read;

}

static unsigned int clone_open(struct system_node *self)
{

    unsigned int index = find_snode();

    if (!index)
        return 0;

    init_snode(&snode[index], index, &inode[1], &pnode[1]);
    system_group_add(&root, &snode[index].base.node);
    system_group_add(&snode[index].base, &snode[index].device.node);
    system_group_add(&snode[index].base, &snode[index].protocol.node);
    system_group_add(&snode[index].base, &snode[index].control.node);

    return (unsigned long)&snode[index].control;

}

static unsigned int data_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
 
    if (offset)
        return 0;

    return node->interface->receive(node->device, count, buffer);

}

static unsigned int data_write(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
 
    if (offset)
        return 0;

    return node->interface->send(node->device, count, buffer);

}

static unsigned int mac_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct interface_node *node = (struct interface_node *)self->parent;
    char *mac = "00:00:00:00:00:00";
    unsigned int i;

    for (i = 0; i < 6; i++)
        ascii_write_zerovalue(mac, 17, node->interface->mac[i], 16, 2, i * 3);

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

static unsigned int find_pnode()
{

    unsigned int i;

    for (i = 1; i < 8; i++)
    {

        if (!pnode[i].base.node.parent)
            return i;

    }

    return 0;

}

static void init_inode(struct interface_node *node, struct base_network_interface *interface, struct base_device *device)
{

    memory_clear(node, sizeof (struct interface_node));
    system_init_group(&node->base, device->module.name);
    system_init_stream(&node->data, "data");
    system_init_stream(&node->mac, "mac");

    node->interface = interface;
    node->device = device;
    node->data.node.read = data_read;
    node->data.node.write = data_write;
    node->mac.node.read = mac_read;

}

static void init_pnode(struct protocol_node *node, struct base_network_protocol *protocol)
{

    memory_clear(node, sizeof (struct protocol_node));
    system_init_group(&node->base, protocol->name);

    node->protocol = protocol;

}

void base_network_register_interface(struct base_network_interface *interface, struct base_device *device)
{

    unsigned int index = find_inode();

    if (!index)
        return;

    init_inode(&inode[index], interface, device);
    system_group_add(&dev, &inode[index].base.node);
    system_group_add(&inode[index].base, &inode[index].data.node);
    system_group_add(&inode[index].base, &inode[index].mac.node);

}

void base_network_register_protocol(struct base_network_protocol *protocol)
{

    unsigned int index = find_pnode();

    if (!index)
        return;

    init_pnode(&pnode[index], protocol);
    system_group_add(&protocols, &pnode[index].base.node);

}

void base_network_init_interface(struct base_network_interface *interface, unsigned int (*receive)(struct base_device *device, unsigned int count, void *buffer), unsigned int (*send)(struct base_device *device, unsigned int count, void *buffer))
{

    memory_clear(interface, sizeof (struct base_network_interface));
    base_init_interface(&interface->base, BASE_INTERFACE_TYPE_OTHER);

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

    memory_clear(inode, sizeof (struct interface_node) * 8);
    memory_clear(pnode, sizeof (struct protocol_node) * 8);
    memory_clear(snode, sizeof (struct session_node) * 8);
    system_init_group(&root, "network");
    system_register_node(&root.node);
    system_init_group(&dev, "dev");
    system_group_add(&root, &dev.node);
    system_init_group(&protocols, "protocols");
    system_group_add(&root, &protocols.node);
    system_init_stream(&clone, "clone");
    system_group_add(&root, &clone.node);

    clone.node.open = clone_open;

}

