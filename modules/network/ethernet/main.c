#include <fudge.h>
#include <kernel.h>
#include <net/ethernet.h>
#include <system/system.h>
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

    scheduler_mailboxes_send(&interface->data.mailboxes, count, buffer);
    event_notify(EVENT_TYPE_NETWORK, count, buffer);

}

static unsigned int interfacenode_ctrlread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct ethernet_interface *interface = (struct ethernet_interface *)self->parent;

    return memory_read(buffer, count, &interface->settings, sizeof (struct ctrl_networksettings), offset);

}

static unsigned int interfacenode_ctrlwrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct ethernet_interface *interface = (struct ethernet_interface *)self->parent;

    return memory_write(&interface->settings, sizeof (struct ctrl_networksettings), buffer, count, offset);

}

void ethernet_registerinterface(struct ethernet_interface *interface, unsigned int id)
{

    system_addchild(&interface->root, &interface->ctrl);
    system_addchild(&interface->root, &interface->data);
    system_addchild(&root, &interface->root);

    interface->id = id;

}

void ethernet_registerprotocol(struct ethernet_protocol *protocol)
{

    resource_register(&protocol->resource);
    list_add(&protocols, &protocol->item);
    system_addchild(&protocol->root, &protocol->data);
    system_addchild(&root, &protocol->root);

}

void ethernet_unregisterinterface(struct ethernet_interface *interface)
{

    system_removechild(&interface->root, &interface->ctrl);
    system_removechild(&interface->root, &interface->data);
    system_removechild(&root, &interface->root);

}

void ethernet_unregisterprotocol(struct ethernet_protocol *protocol)
{

    resource_unregister(&protocol->resource);
    list_remove(&protocols, &protocol->item);
    system_removechild(&protocol->root, &protocol->data);
    system_removechild(&root, &protocol->root);

}

void ethernet_initinterface(struct ethernet_interface *interface, const char *name, unsigned int (*send)(unsigned int count, void *buffer))
{

    system_initnode(&interface->root, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, name);
    system_initnode(&interface->ctrl, SYSTEM_NODETYPE_NORMAL, "ctrl");
    system_initnode(&interface->data, SYSTEM_NODETYPE_MAILBOX, "data");
    ctrl_init_networksettings(&interface->settings);

    interface->send = send;
    interface->ctrl.read = interfacenode_ctrlread;
    interface->ctrl.write = interfacenode_ctrlwrite;

}

void ethernet_initprotocol(struct ethernet_protocol *protocol, const char *name, unsigned short type, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer))
{

    resource_init(&protocol->resource, RESOURCE_PROTONET, protocol);
    list_inititem(&protocol->item, protocol);

    protocol->type = type;
    protocol->notify = notify;

    system_initnode(&protocol->root, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&protocol->data, SYSTEM_NODETYPE_MAILBOX, "data");

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

