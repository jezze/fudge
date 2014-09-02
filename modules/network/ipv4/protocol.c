#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>
#include "ipv4.h"

static unsigned int protocol_read(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int protocol_write(struct base_network_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void ipv4_initprotocol(struct base_network_protocol *protocol)
{

    base_network_initprotocol(protocol, "ipv4", protocol_read, protocol_write);

}

