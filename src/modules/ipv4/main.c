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

static struct ipv4_arpentry *findarpentrybypaddress(void *paddress)
{

    unsigned int i;

    for (i = 0; i < ARPTABLESIZE; i++)
    {

        if (memory_match(arptable[i].paddress, paddress, IPV4_ADDRSIZE))
            return &arptable[i];

    }

    return 0;

}

static struct ipv4_arpentry *findarpentrybyhaddress(void *haddress)
{

    unsigned int i;

    for (i = 0; i < ARPTABLESIZE; i++)
    {

        if (memory_match(arptable[i].haddress, haddress, ETHERNET_ADDRSIZE))
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

void *ipv4_writeheader(void *buffer, unsigned char *sip, unsigned char *tip, unsigned int protocol)
{

    struct ipv4_header *header = buffer;
    unsigned int checksum;

    header->version = 0x45;
    header->dscp = 0;
    header->length[0] = 20;
    header->length[1] = 0;
    header->identification[0] = 0;
    header->identification[1] = 0;
    header->fragment[0] = 0;
    header->fragment[1] = 0;
    header->ttl = 10;
    header->protocol = protocol;
    header->checksum[0] = 0;
    header->checksum[1] = 0;

    memory_copy(header->sip, sip, IPV4_ADDRSIZE);
    memory_copy(header->tip, tip, IPV4_ADDRSIZE);

    checksum = calculatechecksum(header, sizeof (struct ipv4_header));

    header->checksum[0] = checksum & 0xFF;
    header->checksum[1] = (checksum >> 8) & 0xFF;

    return header + 1;

}

void *ipv4_writedata(void *buffer, void *payload, unsigned int count)
{

    unsigned char *data = buffer;

    memory_copy(buffer, payload, count);

    return data + count;

}

void ipv4_send(unsigned char *tip, unsigned int protocol, void *payload, unsigned int count)
{

    unsigned char response[ETHERNET_MTU];
    struct resource *resource;
    struct ethernet_interface *interface;
    struct ipv4_header *header;
    struct ipv4_arpentry *sentry;
    struct ipv4_arpentry *tentry;
    unsigned char *data;
    unsigned char *responsetotal;

    /* Should be through routing */
    resource = resource_findtype(0, RESOURCE_ETHERNETINTERFACE);

    if (!resource)
        return;

    interface = resource->data;

    if (!interface)
        return;

    sentry = findarpentrybyhaddress(interface->haddress);

    if (!sentry)
        return;

    tentry = findarpentrybypaddress(tip);

    if (!tentry)
        return;

    header = ethernet_writeheader(response, ethernetprotocol.type, interface->haddress, tentry->haddress);
    data = ipv4_writeheader(header, sentry->paddress, tentry->paddress, protocol);
    responsetotal = ipv4_writedata(data, payload, count);

    interface->send(response, responsetotal - response);

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

    struct ipv4_arpentry *entry = findarpentrybypaddress(paddress);

    return (entry) ? entry->haddress : 0;

}

static void arphook_save(void *haddress, void *paddress)
{

    struct ipv4_arpentry *entry = findarpentrybypaddress(paddress);

    if (entry)
        return;

    entry = findarpentrybypaddress("\0\0\0\0");

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

