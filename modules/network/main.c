#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>
#include "network.h"

static struct system_node root;

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_interfacenode *node = (struct network_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->ctrl, sizeof (struct ctrl_networkctrl), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_interfacenode *node = (struct network_interfacenode *)self->parent;

    return memory_write(&node->interface->ctrl, sizeof (struct ctrl_networkctrl), buffer, count, offset);

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_interfacenode *node = (struct network_interfacenode *)self->parent;
 
    return node->interface->receive(count, buffer);

}

static unsigned int interfacenode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_interfacenode *node = (struct network_interfacenode *)self->parent;
 
    return node->interface->send(count, buffer);

}

static unsigned int channelnode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_channelnode *node = (struct network_channelnode *)self->parent;
 
    return node->channel->rdata(offset, count, buffer);

}

static unsigned int channelnode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_channelnode *node = (struct network_channelnode *)self->parent;
 
    return node->channel->wdata(offset, count, buffer);

}

void network_notify(struct network_interface *interface)
{

    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_TYPE_CHANNELNET)))
    {

        struct network_channel *channel = current->data;

        if (channel->match(interface))
            channel->notify(interface);

    }

}

void network_registerinterface(struct network_interface *interface)
{

    base_registerinterface(&interface->base);

}

void network_registerinterfacenode(struct network_interfacenode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->ctrl);
    system_addchild(&node->base, &node->data);

}

void network_registerprotocol(struct network_protocol *protocol)
{

    resource_register(&protocol->resource);

}

void network_registerchannel(struct network_channel *channel)
{

    resource_register(&channel->resource);

}

void network_registerchannelnode(struct network_channelnode *node)
{

    system_addchild(&root, &node->base);
    system_addchild(&node->base, &node->data);

}

void network_unregisterinterface(struct network_interface *interface)
{

    base_unregisterinterface(&interface->base);

}

void network_unregisterinterfacenode(struct network_interfacenode *node)
{

    system_removechild(&node->base, &node->ctrl);
    system_removechild(&node->base, &node->data);
    system_removechild(&root, &node->base);

}

void network_unregisterprotocol(struct network_protocol *protocol)
{

    resource_unregister(&protocol->resource);

}

void network_unregisterchannel(struct network_channel *channel)
{

    resource_unregister(&channel->resource);

}

void network_unregisterchannelnode(struct network_channelnode *node)
{

    system_removechild(&node->base, &node->data);
    system_removechild(&root, &node->base);

}

void network_initinterface(struct network_interface *interface, struct base_bus *bus, unsigned int id, unsigned int (*receive)(unsigned int count, void *buffer), unsigned int (*send)(unsigned int count, void *buffer), void *(*getpacket)(), void (*dumppacket)())
{

    memory_clear(interface, sizeof (struct network_interface));
    base_initinterface(&interface->base, bus, id);

    interface->receive = receive;
    interface->send = send;
    interface->getpacket = getpacket;
    interface->dumppacket = dumppacket;

}

void network_initinterfacenode(struct network_interfacenode *node, struct network_interface *interface)
{

    memory_clear(node, sizeof (struct network_interfacenode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, interface->base.bus->name);
    system_initnode(&node->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->interface = interface;
    node->ctrl.read = interfacenode_ctrlread;
    node->ctrl.write = interfacenode_ctrlwrite;
    node->data.read = interfacenode_dataread;
    node->data.write = interfacenode_datawrite;

}

void network_initprotocol(struct network_protocol *protocol, char *name, unsigned int (*read)(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*write)(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer), unsigned int (*match)(struct network_interface *interface))
{

    memory_clear(protocol, sizeof (struct network_protocol));
    resource_init(&protocol->resource, RESOURCE_TYPE_PROTONET, protocol);

    protocol->name = name;
    protocol->match = match;
    protocol->read = read;
    protocol->write = write;

}

void network_initchannel(struct network_channel *channel, unsigned int (*match)(struct network_interface *interface), void (*notify)(struct network_interface *interface), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    memory_clear(channel, sizeof (struct network_channel));
    resource_init(&channel->resource, RESOURCE_TYPE_CHANNELNET, channel);

    channel->match = match;
    channel->notify = notify;
    channel->rdata = rdata;
    channel->wdata = wdata;

}

void network_initchannelnode(struct network_channelnode *node, struct network_channel *channel)
{

    memory_clear(node, sizeof (struct network_channelnode));
    system_initnode(&node->base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "channel");
    system_initnode(&node->data, SYSTEM_NODETYPE_NORMAL, "data");

    node->channel = channel;
    node->data.read = channelnode_dataread;
    node->data.write = channelnode_datawrite;

}

void init()
{

    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "network");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

