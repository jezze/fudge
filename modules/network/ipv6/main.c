#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <network/network.h>

static struct network_protocol protocol;

void init()
{

    network_initprotocol(&protocol, "ipv6");
    network_registerprotocol(&protocol);

}

void destroy()
{

    network_unregisterprotocol(&protocol);

}

