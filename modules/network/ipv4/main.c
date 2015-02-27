#include <fudge.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <kernel.h>
#include <system/system.h>
#include <network/ethernet/ethernet.h>
#include <network/arp/arp.h>
#include "ipv4.h"

#define LOCALS                          8
#define REMOTES                         32

static struct ethernet_protocol ethernetprotocol;
static struct arp_hook arphook;
static struct ipv4_ethernetentry localbuffer[LOCALS];
static unsigned int localbuffercount;
static struct ipv4_ethernetentry remotebuffer[REMOTES];
static unsigned int remotebuffercount;
static struct buffer local;
static struct buffer remote;
static struct system_node localnode;
static struct system_node remotenode;

static void addlocalentry(unsigned char *ha, unsigned char *pa)
{

    memory_copy(localbuffer[localbuffercount].ha, ha, 6);
    memory_copy(localbuffer[localbuffercount].pa, pa, 4);

    localbuffercount++;

}

static void addremoteentry(unsigned char *ha, unsigned char *pa)
{

    memory_copy(remotebuffer[remotebuffercount].ha, ha, 6);
    memory_copy(remotebuffer[remotebuffercount].pa, pa, 4);

    remotebuffercount++;

}

static void ethernetprotocol_addinterface(struct ethernet_interface *interface)
{

    unsigned char pa[4] = {192, 168, 0, 100};

    addlocalentry(interface->mac, pa);

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&ethernetprotocol.data.mailboxes, count, buffer);

}

static unsigned int localnode_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, localbuffer, sizeof (struct ipv4_ethernetentry) * LOCALS, offset);

}

static unsigned int remotenode_read(struct system_node *self, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, remotebuffer, sizeof (struct ipv4_ethernetentry) * REMOTES, offset);

}

static void arphook_notify(unsigned int count, void *buffer)
{

    struct {struct arp_header header; struct ipv4_ethernetentry source; struct ipv4_ethernetentry target;} *message = buffer;
    unsigned int i;

    /* ugly workaround */
    if (!remotebuffercount)
        addremoteentry(message->source.ha, message->source.pa);

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
    ethernet_initprotocol(&ethernetprotocol, "ipv4", 0x0800, ethernetprotocol_addinterface, ethernetprotocol_notify);
    arp_inithook(&arphook, 0x0001, ethernetprotocol.type, arphook_notify);
    system_initnode(&localnode, SYSTEM_NODETYPE_NORMAL, "local");
    system_initnode(&remotenode, SYSTEM_NODETYPE_NORMAL, "remote");

    localnode.read = localnode_read;
    remotenode.read = remotenode_read;

}

void module_register()
{

    ethernet_registerprotocol(&ethernetprotocol);
    arp_registerhook(&arphook);
    system_addchild(&ethernetprotocol.root, &localnode);
    system_addchild(&ethernetprotocol.root, &remotenode);

}

void module_unregister()
{

    ethernet_unregisterprotocol(&ethernetprotocol);
    arp_unregisterhook(&arphook);
    system_removechild(&ethernetprotocol.root, &localnode);
    system_removechild(&ethernetprotocol.root, &remotenode);

}

