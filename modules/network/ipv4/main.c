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

    scheduler_mailboxes_send(&protocol.node.data.mailboxes, count, buffer);

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

    list_inititem(&protocol->item, protocol);

    protocol->id = id;
    protocol->notify = notify;

    system_initnode(&protocol->node.base, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&protocol->node.data, SYSTEM_NODETYPE_MAILBOX, "data");

    protocol->node.protocol = protocol;

}

void module_init()
{

    ethernet_initprotocol(&protocol, "ipv4", 0x0800, protocol_notify);
    arp_inithook(&arphook, protocol.type, arphook_notify);

}

void module_register()
{

    ethernet_registerprotocol(&protocol);
    arp_registerhook(&arphook);

}

void module_unregister()
{

    ethernet_unregisterprotocol(&protocol);
    arp_unregisterhook(&arphook);

}

