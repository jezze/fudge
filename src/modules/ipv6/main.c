#include <fudge.h>
#include <net/ipv6.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include "ipv6.h"

static struct ethernet_protocol ethernetprotocol;

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    system_multicast(&ethernetprotocol.data.links, count, buffer);

}

void module_init(void)
{

    ethernet_initprotocol(&ethernetprotocol, "ipv6", IPV6_PROTOCOL, ethernetprotocol_notify);

}

void module_register(void)
{

    ethernet_registerprotocol(&ethernetprotocol);

}

void module_unregister(void)
{

    ethernet_unregisterprotocol(&ethernetprotocol);

}

