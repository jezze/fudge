#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arp/arp.h>
#include "ipv4.h"

#define ARPTABLESIZE                    8

static struct ethernet_hook ethernethook;
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

unsigned int ipv4_calculatechecksum(void *buffer, unsigned int count)
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

    header = ethernet_writehead(buffer, ethernethook.type, sentry->haddress, tentry->haddress);

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

    checksum = ipv4_calculatechecksum(header, sizeof (struct ipv4_header));

    header->checksum[0] = checksum;
    header->checksum[1] = checksum >> 8;

    return header + 1;

}

void ipv4_send(void *buffer, unsigned int count)
{

    ethernet_send(buffer, count);

}

static void ethernethook_notify(struct ethernet_header *ethernetheader, void *buffer, unsigned int count)
{

    struct ipv4_header *header = buffer;
    unsigned int length = (header->length[0] << 8) | header->length[1];
    struct resource *current = 0;

    while ((current = resource_foreachtype(current, RESOURCE_IPV4HOOK)))
    {

        struct ipv4_hook *hook = current->data;

        if (hook->id == header->protocol)
            hook->notify(header, header + 1, length - sizeof (struct ipv4_header));

    }

    kernel_notify(&ethernethook.data.links, EVENT_DATA, buffer, count);

}

static unsigned int arptablenode_read(void *buffer, unsigned int count, unsigned int offset)
{

    return memory_read(buffer, count, arptable, sizeof (struct ipv4_arpentry) * ARPTABLESIZE, offset);

}

static unsigned int arptablenode_write(void *buffer, unsigned int count, unsigned int offset)
{

    return memory_write(arptable, sizeof (struct ipv4_arpentry) * ARPTABLESIZE, buffer, count, offset);

}

static unsigned int arphook_match(unsigned int htype, unsigned char hlength, unsigned int ptype, unsigned char plength)
{

    return htype == 1 && hlength == ETHERNET_ADDRSIZE && ptype == ethernethook.type && plength == IPV4_ADDRSIZE;

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

void ipv4_registerhook(struct ipv4_hook *hook)
{

    resource_register(&hook->resource);
    system_addchild(&hook->root, &hook->data);
    system_addchild(&ethernethook.root, &hook->root);

}

void ipv4_unregisterhook(struct ipv4_hook *hook)
{

    resource_unregister(&hook->resource);
    system_removechild(&hook->root, &hook->data);
    system_removechild(&ethernethook.root, &hook->root);

}

void ipv4_inithook(struct ipv4_hook *hook, char *name, unsigned char id, void (*notify)(struct ipv4_header *ipv4header, void *buffer, unsigned int count))
{

    resource_init(&hook->resource, RESOURCE_IPV4HOOK, hook);
    system_initnode(&hook->root, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&hook->data, SYSTEM_NODETYPE_NORMAL, "data");

    hook->id = id;
    hook->notify = notify;

}

void module_init(void)
{

    ethernet_inithook(&ethernethook, "ipv4", 0x0800, ethernethook_notify);
    arp_inithook(&arphook, arphook_match, arphook_lookup, arphook_save);
    system_initnode(&arptablenode, SYSTEM_NODETYPE_NORMAL, "arptable");

    arptablenode.operations.read = arptablenode_read;
    arptablenode.operations.write = arptablenode_write;

}

void module_register(void)
{

    ethernet_registerhook(&ethernethook);
    arp_registerhook(&arphook);
    system_addchild(&ethernethook.root, &arptablenode);

}

void module_unregister(void)
{

    ethernet_unregisterhook(&ethernethook);
    arp_unregisterhook(&arphook);
    system_removechild(&ethernethook.root, &arptablenode);

}

