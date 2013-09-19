#include <fudge/kernel.h>
#include <base/base.h>
#include <base/network.h>
#include <net/net.h>
#include "arp.h"

static unsigned int read(struct base_network *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write(struct base_network *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void arp_init_protocol(struct net_protocol *protocol)
{

    net_init_protocol(protocol, "arp", read, write);

}

