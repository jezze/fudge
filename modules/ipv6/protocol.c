#include <fudge/module.h>
#include <net/net.h>
#include "ipv6.h"

static unsigned int read(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void ipv6_init_protocol(struct ipv6_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct ipv6_protocol));
    net_init_protocol(&protocol->base, "ipv6", read, write);

}

