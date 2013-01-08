#include <fudge/kernel.h>
#include <system/system.h>
#include <net/net.h>
#include "arp.h"

unsigned int read(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

unsigned int write(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void arp_init_protocol(struct arp_protocol *protocol)
{

    memory_clear(protocol, sizeof (struct arp_protocol));

    net_init_protocol(&protocol->base, "arp", read, write);

}

