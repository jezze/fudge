#include <fudge.h>
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

void *ipv4_writeheader(void *buffer, unsigned char *sip, unsigned char *tip)
{

    struct ipv4_header *header = buffer;

    memory_copy(header->sip, sip, IPV4_ADDRSIZE);
    memory_copy(header->tip, tip, IPV4_ADDRSIZE);

    return header + 1;

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, struct ethernet_header *ethernetheader, void *buffer, unsigned int count)
{

    struct ipv4_header *header = buffer;
    unsigned int length = (header->length[0] << 8) | header->length[1];
    struct resource *current = 0;

    while ((current = resource_findtype(current, RESOURCE_IPV4PROTOCOL)))
    {

        struct ipv4_protocol *protocol = current->data;

        if (protocol->id == header->protocol)
            protocol->notify(interface, header, header + 1, length - sizeof (struct ipv4_header));

    }

    kernel_multicast(&ethernetprotocol.datalinks, buffer, count);

}

static unsigned int arptablenode_read(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return memory_read(buffer, count, arptable, sizeof (struct ipv4_arpentry) * ARPTABLESIZE, state->offset);

}

static unsigned int arptablenode_write(struct system_node *self, struct service_state *state, void *buffer, unsigned int count)
{

    return memory_write(arptable, sizeof (struct ipv4_arpentry) * ARPTABLESIZE, buffer, count, state->offset);

}

static unsigned int arphook_match(unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength)
{

    return htype == 1 && hlength == ETHERNET_ADDRSIZE && ptype == ethernetprotocol.type && plength == IPV4_ADDRSIZE;

}

static struct ipv4_arpentry *findarpentry(void *paddress)
{

    unsigned int i;

    for (i = 0; i < ARPTABLESIZE; i++)
    {

        if (memory_match(arptable[i].paddress, paddress, IPV4_ADDRSIZE))
            return &arptable[i];

    }

    return 0;

}

static unsigned char *arphook_lookup(void *paddress)
{

    struct ipv4_arpentry *entry = findarpentry(paddress);

    return (entry) ? entry->haddress : 0;

}

static void arphook_save(void *haddress, void *paddress)
{

    struct ipv4_arpentry *entry = findarpentry(paddress);

    if (entry)
        return;

    entry = findarpentry("\0\0\0\0");

    if (!entry)
        return;

    memory_copy(entry->haddress, haddress, ETHERNET_ADDRSIZE);
    memory_copy(entry->paddress, paddress, IPV4_ADDRSIZE);

}

static unsigned int protocoldata_open(struct system_node *self, struct service_state *state)
{

    struct ipv4_protocol *protocol = self->resource->data;

    list_add(&protocol->datalinks, &state->link);

    return state->id;

}

static unsigned int protocoldata_close(struct system_node *self, struct service_state *state)
{

    struct ipv4_protocol *protocol = self->resource->data;

    list_remove(&protocol->datalinks, &state->link);

    return state->id;

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

void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ethernet_interface *interface, struct ipv4_header *header, void *buffer, unsigned int count))
{

    resource_init(&protocol->resource, RESOURCE_IPV4PROTOCOL, protocol);
    system_initresourcenode(&protocol->root, SYSTEM_NODETYPE_GROUP, name, &protocol->resource);
    system_initresourcenode(&protocol->data, SYSTEM_NODETYPE_NORMAL, "data", &protocol->resource);

    protocol->id = id;
    protocol->notify = notify;
    protocol->data.open = protocoldata_open;
    protocol->data.close = protocoldata_close;
    protocol->data.read = system_readtask;

}

void module_init(void)
{

    ethernet_initprotocol(&ethernetprotocol, "ipv4", IPV4_PROTOCOL, ethernetprotocol_notify);
    arp_inithook(&arphook, arphook_match, arphook_lookup, arphook_save);
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

