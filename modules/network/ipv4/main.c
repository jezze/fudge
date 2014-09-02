#include <module.h>
#include <kernel/resource.h>
#include <system/system.h>
#include <base/base.h>
#include <base/network.h>
#include "ipv4.h"

static struct base_network_protocol protocol;

void init()
{

    ipv4_initprotocol(&protocol);
    base_network_registerprotocol(&protocol);

}

void destroy()
{

}

