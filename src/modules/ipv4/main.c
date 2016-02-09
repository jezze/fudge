#include <fudge.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/arp/arp.h>
#include "ipv4.h"

#define LOCALS                          8

static struct ethernet_protocol ethernetprotocol;
static struct arp_hook arphook;
static struct ipv4_ethernetentry localbuffer[LOCALS];
static unsigned int localbuffercount;
static struct buffer local;
static struct system_node localnode;

static void addlocalentry(unsigned char *hardwareaddress, unsigned char *protocoladdress)
{

    memory_copy(localbuffer[localbuffercount].hardwareaddress, hardwareaddress, 6);
    memory_copy(localbuffer[localbuffercount].protocoladdress, protocoladdress, 4);

    localbuffercount++;

}

static void ethernetprotocol_addinterface(struct ethernet_interface *interface)
{

    unsigned char protocoladdress[4] = {192, 168, 0, 100};

    addlocalentry(interface->hardwareaddress, protocoladdress);

}

static void ethernetprotocol_removeinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    system_multicast(&ethernetprotocol.data.links, count, buffer);

}

static unsigned int localnode_read(struct system_node *self, struct task *task, unsigned int descriptor, unsigned int offset, unsigned int count, void *buffer)
{

    return memory_read(buffer, count, localbuffer, sizeof (struct ipv4_ethernetentry) * LOCALS, offset);

}

static unsigned char *arphook_gethardwareaddress(unsigned int count, void *protocoladdress)
{

    unsigned int i;

    for (i = 0; i < LOCALS; i++)
    {

        if (memory_match(localbuffer[i].protocoladdress, protocoladdress, count))
            return localbuffer[i].hardwareaddress;

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
    system_initnode(&protocol->root, SYSTEM_NODETYPE_GROUP, name);
    system_initnode(&protocol->data, SYSTEM_NODETYPE_MAILBOX, "data");

    protocol->id = id;
    protocol->notify = notify;
    protocol->data.resource = &protocol->resource;

}

void module_init(void)
{

    buffer_init(&local, sizeof (struct ipv4_ethernetentry) * LOCALS, &localbuffer);
    ethernet_initprotocol(&ethernetprotocol, "ipv4", 0x0800, ethernetprotocol_addinterface, ethernetprotocol_removeinterface, ethernetprotocol_notify);
    arp_inithook(&arphook, 0x0001, ethernetprotocol.type, arphook_gethardwareaddress);
    system_initnode(&localnode, SYSTEM_NODETYPE_NORMAL, "local");

    localnode.read = localnode_read;

}

void module_register(void)
{

    ethernet_registerprotocol(&ethernetprotocol);
    arp_registerhook(&arphook);
    system_addchild(&ethernetprotocol.root, &localnode);

}

void module_unregister(void)
{

    ethernet_unregisterprotocol(&ethernetprotocol);
    arp_unregisterhook(&arphook);
    system_removechild(&ethernetprotocol.root, &localnode);

}

