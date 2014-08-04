#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>
#include "arp.h"

static struct base_network_protocol protocol;

void init()
{

    arp_init_protocol(&protocol);
    base_network_register_protocol(&protocol);

}

void destroy()
{

}

