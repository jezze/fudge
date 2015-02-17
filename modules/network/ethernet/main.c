#include <fudge.h>
#include <kernel.h>
#include <net/ethernet.h>
#include <system/system.h>
#include <base/base.h>
#include <event/event.h>
#include "ethernet.h"

static struct system_node root;
static struct list protocols;

void ethernet_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    struct ethernet_header *header = buffer;
    unsigned short type = (header->type[0] << 8) | header->type[1];
    struct list_item *current;

    for (current = protocols.head; current; current = current->next)
    {

        struct ethernet_protocol *protocol = current->data;

        if (protocol->type == type)
            protocol->notify(interface, count - 18, header + 1);

    }

    scheduler_mailboxes_send(&interface->node.data.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_NETWORK, count, buffer);

}

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct ethernet_interfacenode *node = (struct ethernet_interfacenode *)self->parent;

    return memory_read(buffer, count, &node->interface->settings, sizeof (struct ctrl_networksettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct ethernet_interfacenode *node = (struct ethernet_interfacenode *)self->parent;

    return memory_write(&node->interface->settings, sizeof (struct ctrl_networksettings), buffer, count, offset);

}

void ethernet_registerinterface(struct ethernet_interface *interface, unsigned int id)
{

    base_registerinterface(&interface->base, id);
    system_addchild(&root, &interface->node.base);
    system_addchild(&interface->node.base, &interface->node.ctrl);
    system_addchild(&interface->node.base, &interface->node.data);

}

void ethernet_registerprotocol(struct ethernet_protocol *protocol)
{

    resource_register(&protocol->resource);
    list_add(&protocols, &protocol->item);
    system_addchild(&root, &protocol->node.base);
    system_addchild(&protocol->node.base, &protocol->node.data);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    base_unregisterinterface(&interface->base);
    system_removechild(&interface->node.base, &interface->node.ctrl);
    system_removechild(&interface->node.base, &interface->node.data);
    system_removechild(&root, &interface->node.base);

}

void ethernet_unregisterprotocol(struct ethernet_protocol *protocol)
{

    resource_unregister(&protocol->resource);
    list_remove(&protocols, &protocol->item);
    system_removechild(&protocol->node.base, &protocol->node.data);
    system_removechild(&root, &protocol->node.base);

}

void ethernet_initinterface(struct ethernet_interface *interface, struct base_driver *driver, unsigned int (*send)(unsigned int count, void *buffer))
{

    base_initinterface(&interface->base, driver);
    ctrl_init_networksettings(&interface->settings);

    interface->send = send;

    system_initnode(&interface->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, driver->name);
    system_initnode(&interface->node.ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->node.data, SYSTEM_NODETYPE_MAILBOX, "data");

    interface->node.interface = interface;
    interface->node.ctrl.read = interfacenode_ctrlread;
    interface->node.ctrl.write = interfacenode_ctrlwrite;

}

void ethernet_initprotocol(struct ethernet_protocol *protocol, char *name, unsigned short type, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer))
{

    resource_init(&protocol->resource, RESOURCE_TYPE_PROTONET, protocol);
    list_inititem(&protocol->item, protocol);

    protocol->type = type;
    protocol->notify = notify;

    system_initnode(&protocol->node.base, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&protocol->node.data, SYSTEM_NODETYPE_MAILBOX, "data");

    protocol->node.protocol = protocol;

}

void module_init()
{

    list_init(&protocols);
    system_initnode(&root, SYSTEM_NODETYPE_GROUP, "ethernet");

}

void module_register()
{

    system_registernode(&root);

}

void module_unregister()
{

    system_unregisternode(&root);

}

