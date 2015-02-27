#include <fudge.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <kernel.h>
#include <system/system.h>
#include <network/ethernet/ethernet.h>
#include <network/arp/arp.h>
#include "ipv4.h"

#define LOCALS                          8
#define REMOTES                         64

static struct ethernet_protocol ethernetprotocol;
static struct arp_hook arphook;
static struct ipv4_ethernetentry localbuffer[LOCALS];
static struct ipv4_ethernetentry remotebuffer[REMOTES];
static struct buffer local;
static struct buffer remote;

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&ethernetprotocol.data.mailboxes, count, buffer);

}

static void arphook_notify(unsigned int count, void *buffer)
{

    struct {struct arp_header header; struct ipv4_ethernetentry sender; struct ipv4_ethernetentry target;} *message = buffer;
    unsigned int i;

    for (i = 0; i < LOCALS; i++)
    {

        if (memory_match(&localbuffer[i], &message->target, 10))
            return;

    }

    return;

}

void ipv4_registerprotocol(struct ipv4_protocol *protocol)
{

    resource_register(&protocol->resource);
    system_addchild(&protocol->root, &protocol->data);
    system_addchild(&ethernetprotocol.root, &protocol->root);

}

void ipv4_unregisterprotocol(struct ipv4_protocol *protocol)
{

    resource_unregister(&protocol->resource);
    system_removechild(&protocol->root, &protocol->data);
    system_removechild(&ethernetprotocol.root, &protocol->root);

}

void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, unsigned int count, void *buffer))
{

    resource_init(&protocol->resource, RESOURCE_IPV4PROTOCOL, protocol);
    system_initnode(&protocol->root, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&protocol->data, SYSTEM_NODETYPE_MAILBOX, "data");

    protocol->id = id;
    protocol->notify = notify;
    protocol->data.resource = &protocol->resource;

}

void module_init()
{

    buffer_init(&local, sizeof (struct ipv4_ethernetentry), sizeof (struct ipv4_ethernetentry) * LOCALS, &localbuffer);
    buffer_init(&remote, sizeof (struct ipv4_ethernetentry), sizeof (struct ipv4_ethernetentry) * REMOTES, &remotebuffer);
    ethernet_initprotocol(&ethernetprotocol, "ipv4", 0x0800, ethernetprotocol_notify);
    arp_inithook(&arphook, 0x0001, ethernetprotocol.type, arphook_notify);

}

void module_register()
{

    ethernet_registerprotocol(&ethernetprotocol);
    arp_registerhook(&arphook);

}

void module_unregister()
{

    ethernet_unregisterprotocol(&ethernetprotocol);
    arp_unregisterhook(&arphook);

}

