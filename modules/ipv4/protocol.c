#include <fudge/module.h>
#include <net/net.h>
#include "ipv4.h"

static unsigned int read(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

static unsigned int write(struct net_interface *interface, unsigned int offset, unsigned int count, void *buffer)
{

    return 0;

}

void ipv4_init_protocol(struct net_protocol *protocol)
{

    net_init_protocol(protocol, "ipv4", read, write);

}

