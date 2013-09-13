#include <fudge/module.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>
#include <net/net.h>
#include "arp.h"

static struct net_protocol protocol;

void init()
{

    arp_init_protocol(&protocol);
    net_register_protocol(0x0806, &protocol);

}

void destroy()
{

    net_unregister_protocol(0x0806);

}

