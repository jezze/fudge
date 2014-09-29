#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>

static struct base_network_protocol protocol;
static struct base_network_protocolnode protocolnode;

static unsigned int protocol_match(struct base_network_interface *interface)
{

    return 0;

}

static unsigned int protocol_read(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int protocol_write(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void init()
{

    base_network_initprotocol(&protocol, "arp", protocol_read, protocol_write, protocol_match);
    base_network_initprotocolnode(&protocolnode, &protocol);
    base_network_registerprotocol(&protocol);
    base_network_registerprotocolnode(&protocolnode);

}

void destroy()
{

    base_network_unregisterprotocolnode(&protocolnode);
    base_network_unregisterprotocol(&protocol);

}

