#include <fudge.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <network/ethernet/ethernet.h>
#include <network/arp/arp.h>
#include "ipv4.h"

static struct ethernet_protocol protocol;
static struct list protocols;
static struct arp_hook arphook;

static void protocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&protocol.mailboxes, count, buffer);

}

static unsigned int protocolnode_dataopen(struct system_node *self)
{

    struct ipv4_protocolnode *node = (struct ipv4_protocolnode *)self->parent;

    scheduler_mailboxes_addactive(&node->protocol->mailboxes);

    return system_open(self);

}

static unsigned int protocolnode_dataclose(struct system_node *self)
{

    struct ipv4_protocolnode *node = (struct ipv4_protocolnode *)self->parent;

    scheduler_mailboxes_removeactive(&node->protocol->mailboxes);

    return system_close(self);

}

static unsigned int protocolnode_dataread(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    struct ipv4_protocolnode *node = (struct ipv4_protocolnode *)self->parent;

    return scheduler_mailboxes_readactive(&node->protocol->mailboxes, count, buffer);

}

static unsigned int protocolnode_datawrite(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static void arphook_handlerequest(struct ethernet_interface *interface, struct arp_message *message)
{

    unsigned char mcast[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    if (memory_match(message->tha, interface->settings.mac, 6))
    {

    }

    if (memory_match(message->tha, mcast, 6))
    {

    }

}

static void arphook_handlereply(struct ethernet_interface *interface, struct arp_message *message)
{

}

static void arphook_notify(struct ethernet_interface *interface, struct arp_message *message)
{

    unsigned short operation = (message->operation[0] << 8) | message->operation[1];

    switch (operation)
    {

    case 1:
        arphook_handlerequest(interface, message);

        break;

    case 2:
        arphook_handlereply(interface, message);

        break;

    }

}

void ipv4_registerprotocol(struct ipv4_protocol *p)
{

    list_add(&protocols, &p->item);
    system_addchild(&protocol.node.base, &p->node.base);
    system_addchild(&p->node.base, &p->node.data);

}

void ipv4_unregisterprotocol(struct ipv4_protocol *p)
{

    list_remove(&protocols, &p->item);
    system_removechild(&p->node.base, &p->node.data);
    system_removechild(&protocol.node.base, &p->node.base);

}

void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer))
{

    system_initnode(&protocol->node.base, SYSTEM_NODETYPE_GROUP | SYSTEM_NODETYPE_MULTI, name);
    system_initnode(&protocol->node.data, SYSTEM_NODETYPE_NORMAL, "data");
    list_inititem(&protocol->item, protocol);
    list_init(&protocol->mailboxes);

    protocol->id = id;
    protocol->notify = notify;
    protocol->node.protocol = protocol;
    protocol->node.data.open = protocolnode_dataopen;
    protocol->node.data.close = protocolnode_dataclose;
    protocol->node.data.read = protocolnode_dataread;
    protocol->node.data.write = protocolnode_datawrite;

}

void init()
{

    ethernet_initprotocol(&protocol, "ipv4", 0x0800, protocol_notify);
    ethernet_registerprotocol(&protocol);
    arp_inithook(&arphook, protocol.type, arphook_notify);
    arp_registerhook(&arphook);

}

void destroy()
{

    ethernet_unregisterprotocol(&protocol);
    arp_unregisterhook(&arphook);

}

