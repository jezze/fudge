#include <fudge/memory.h>
#include <net/net.h>
#include "ipv4.h"

unsigned int read(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

unsigned int write(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void ipv4_init_protocol(struct ipv4_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct ipv4_protocol));

    net_init_protocol(&protocol->base, "ipv4", read, write);

}

