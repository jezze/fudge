#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <network/network.h>

static struct network_protocol protocol;

static unsigned int protocol_match(struct network_interface *interface)
{

    return 0;

}

static unsigned int protocol_read(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int protocol_write(struct network_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void init()
{

    network_initprotocol(&protocol, "ipv4", protocol_read, protocol_write, protocol_match);
    network_registerprotocol(&protocol);

}

void destroy()
{

    network_unregisterprotocol(&protocol);

}

