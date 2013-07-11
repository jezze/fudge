#include <fudge/module.h>
#include <net/net.h>
#include "ipv6.h"

static struct net_protocol protocol;

void init()
{

    ipv6_init_protocol(&protocol);
    net_register_protocol(0x86DD, &protocol);

}

void destroy()
{

    net_unregister_protocol(0x86DD);

}

