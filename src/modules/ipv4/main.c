#include <fudge.h>
#include <net/ipv4.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arp/arp.h>
#include "ipv4.h"

#define ARPTABLESIZE                    8

static struct ethernet_protocol ethernetprotocol;
static struct arp_hook arphook;
static struct ipv4_arpentry arptable[ARPTABLESIZE];
static struct system_node arptablenode;

unsigned int ipv4_writeheader(unsigned char *sip, unsigned char *tip, void *buffer)
{

    struct ipv4_header *header = buffer;

    memory_copy(header->sip, sip, IPV4_ADDRSIZE);
    memory_copy(header->tip, tip, IPV4_ADDRSIZE);

    return sizeof (struct ipv4_header);

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    struct ipv4_header *header = buffer;
    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_IPV4PROTOCOL)))
    {

        struct ipv4_protocol *protocol = current->data;

        if (protocol->id == header->protocol)
            protocol->notify(interface, count, header);

    }

    system_multicast(&ethernetprotocol.data.links, count, buffer);

}

static unsigned int arptablenode_read(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, arptable, sizeof (struct ipv4_arpentry) * ARPTABLESIZE, state->offset);

}

static unsigned int arptablenode_write(struct system_node *self, struct service_state *state, unsigned int count, void *buffer)
{

    return memory_write(arptable, sizeof (struct ipv4_arpentry) * ARPTABLESIZE, buffer, count, state->offset);

}

static unsigned int arphook_match(unsigned short htype, unsigned char hlength, unsigned short ptype, unsigned char plength)
{

    return htype == 1 && hlength == ETHERNET_ADDRSIZE && ptype == ethernetprotocol.type && plength == IPV4_ADDRSIZE;

}

static unsigned char *arphook_lookup(void *paddress)
{

    unsigned int i;

    for (i = 0; i < ARPTABLESIZE; i++)
    {

        if (memory_match(arptable[i].paddress, paddress, IPV4_ADDRSIZE))
            return arptable[i].haddress;

    }

    return 0;

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
    system_initresourcenode(&protocol->root, SYSTEM_NODETYPE_GROUP, name, &protocol->resource);
    system_initresourcenode(&protocol->data, SYSTEM_NODETYPE_MAILBOX, "data", &protocol->resource);

    protocol->id = id;
    protocol->notify = notify;

}

void module_init(void)
{

    ethernet_initprotocol(&ethernetprotocol, "ipv4", IPV4_PROTOCOL, ethernetprotocol_notify);
    arp_inithook(&arphook, arphook_match, arphook_lookup);
    system_initnode(&arptablenode, SYSTEM_NODETYPE_NORMAL, "arptable");

    arptablenode.read = arptablenode_read;
    arptablenode.write = arptablenode_write;

}

void module_register(void)
{

    ethernet_registerprotocol(&ethernetprotocol);
    arp_registerhook(&arphook);
    system_addchild(&ethernetprotocol.root, &arptablenode);

}

void module_unregister(void)
{

    ethernet_unregisterprotocol(&ethernetprotocol);
    arp_unregisterhook(&arphook);
    system_removechild(&ethernetprotocol.root, &arptablenode);

}

