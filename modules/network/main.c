#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "network.h"

static struct list mailboxes;
static struct system_node root;

void network_notify(unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&mailboxes, count, buffer);
    event_notify(EVENT_TYPE_NETWORK, count, buffer);

}

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_interfacenode *node = (struct network_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->settings, sizeof (struct ctrl_networksettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_interfacenode *node = (struct network_interfacenode *)self->parent;

    return memory_write(&node->interface->settings, sizeof (struct ctrl_networksettings), buffer, count, offset);

}

static unsigned int interfacenode_dataopen(struct system_node *self)
{

    scheduler_mailboxes_addactive(&mailboxes);

    return system_open(self);

}

static unsigned int interfacenode_dataclose(struct system_node *self)
{

    scheduler_mailboxes_removeactive(&mailboxes);

    return system_close(self);

}

static unsigned int interfacenode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return scheduler_mailboxes_readactive(&mailboxes, count, buffer);

}

static unsigned int interfacenode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct network_interfacenode *node = (struct network_interfacenode *)self->parent;

    return node->interface->wdata(count, buffer);

}

void network_registerinterface(struct network_interface *interface, struct base_bus *bus, unsigned int id)
{

    base_registerinterface(&interface->base, bus, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.ctrl);
    system_addchild(&interface->node.base, &interface->node.data);

}

void network_registerprotocol(struct network_protocol *protocol)
{

    resource_register(&protocol->resource);

}

void network_registerchannel(struct network_channel *channel)
{

    resource_register(&channel->resource);
    system_addchild(&root, &channel->node.base);
    system_addchild(&channel->node.base, &channel->node.data);

}

void network_unregisterinterface(struct network_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.ctrl);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void network_unregisterprotocol(struct network_protocol *protocol)
{

    resource_unregister(&protocol->resource);

}

void network_unregisterchannel(struct network_channel *channel)
{

    resource_unregister(&channel->resource);
    system_removechild(&channel->node.base, &channel->node.data);
    system_removechild(&root, &channel->node.base);

}

void network_initinterface(struct network_interface *interface, struct base_driver *driver, unsigned int (*rdata)(unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int count, void *buffer))
{

    base_initinterface(&interface->base, driver);
    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_NORMAL, "data");
    ctrl_init_networksettings(&interface->settings);

    interface->rdata = rdata;
    interface->wdata = wdata;
    interface->node.interface = interface;
    interface->node.ctrl.read = interfacenode_ctrlread;
    interface->node.ctrl.write = interfacenode_ctrlwrite;
    interface->node.data.open = interfacenode_dataopen;
    interface->node.data.close = interfacenode_dataclose;
    interface->node.data.read = interfacenode_dataread;
    interface->node.data.write = interfacenode_datawrite;

}

void network_initprotocol(struct network_protocol *protocol, char *name)
{

    resource_init(&protocol->resource, RESOURCE_TYPE_PROTONET, protocol);

    protocol->name = name;

}

void network_initchannel(struct network_channel *channel, unsigned int (*match)(struct network_interface *interface, void *packet, unsigned int count), void (*notify)(struct network_interface *interface, void *packet, unsigned int count), unsigned int (*rdata)(unsigned int offset, unsigned int count, void *buffer), unsigned int (*wdata)(unsigned int offset, unsigned int count, void *buffer))
{

    resource_init(&channel->resource, RESOURCE_TYPE_CHANNELNET, channel);
    system_initnode(&channel->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, "channel");
    system_initnode(&channel->node.data, SYSTEM_NODETYPE_NORMAL, "data");

    channel->match = match;
    channel->notify = notify;
    channel->rdata = rdata;
    channel->wdata = wdata;
    channel->node.channel = channel;

}

void init()
{

    list_init(&mailboxes);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "network");
    system_registernode(&root);

}

void destroy()
{

    system_unregisternode(&root);

}

