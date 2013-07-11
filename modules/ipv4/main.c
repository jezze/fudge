#include <fudge/module.h>
#include <net/net.h>
#include "ipv4.h"

static struct net_protocol protocol;

void init()
{

    ipv4_init_protocol(&protocol);
    net_register_protocol(0x0800, &protocol);

}

void destroy()
{

    net_unregister_protocol(0x0800);

}

