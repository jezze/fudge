#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arp/arp.h>
#include "ipv4.h"

#define ARPTABLESIZE                    8

struct ipv4_arpentry
{

    unsigned char haddress[ETHERNET_ADDRSIZE];
    unsigned char paddress[IPV4_ADDRSIZE];

};

static struct ethernet_protocol ethernetprotocol;
static struct arp_hook arphook;
static struct ipv4_arpentry arptable[ARPTABLESIZE];
static struct system_node arptablenode;

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

static unsigned int calculatechecksum(void *buffer, unsigned int count)
{

    unsigned short *ip1 = buffer;
    unsigned int sum = 0;

    while (count > 1)
    {

        sum += *ip1++;

        if (sum & 0x80000000)
            sum = (sum & 0xFFFF) + (sum >> 16);

        count -= 2;

    }

    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);

    return ~sum;

}

void *ipv4_writehead(void *buffer, unsigned char *sip, unsigned char *tip, unsigned int protocol, unsigned int count)
{

    struct ipv4_arpentry *sentry = findarpentry(sip);
    struct ipv4_arpentry *tentry = findarpentry(tip);
    struct ipv4_header *header;
    unsigned int checksum;

    if (!sentry || !tentry)
        return 0;

    header = ethernet_writehead(buffer, ethernetprotocol.type, sentry->haddress, tentry->haddress);

    if (!header)
        return 0;

    header->version = 0x45;
    header->dscp = 0;
    header->length[0] = (count + 20) >> 8;
    header->length[1] = (count + 20);
    header->identification[0] = 0;
    header->identification[1] = 0;
    header->fragment[0] = 0;
    header->fragment[1] = 0;
    header->ttl = 64;
    header->protocol = protocol;
    header->checksum[0] = 0;
    header->checksum[1] = 0;
    header->sip[0] = sip[0];
    header->sip[1] = sip[1];
    header->sip[2] = sip[2];
    header->sip[3] = sip[3];
    header->tip[0] = tip[0];
    header->tip[1] = tip[1];
    header->tip[2] = tip[2];
    header->tip[3] = tip[3];

    checksum = calculatechecksum(header, sizeof (struct ipv4_header));

    header->checksum[0] = checksum;
    header->checksum[1] = checksum >> 8;

    return header + 1;

}

void ipv4_send(void *buffer, unsigned int count)
{

    ethernet_send(buffer, count);

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
            protocol->notify(header, header + 1, length - sizeof (struct ipv4_header));

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

void ipv4_initprotocol(struct ipv4_protocol *protocol, char *name, unsigned char id, void (*notify)(struct ipv4_header *header, void *buffer, unsigned int count))
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

