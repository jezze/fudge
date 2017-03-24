#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/ipv4/ipv4.h>
#include <modules/ipv6/ipv6.h>

static struct ipv4_protocol ipv4protocol;
static struct ipv6_protocol ipv6protocol;

static void ipv4protocol_notify(struct ipv4_header *ipv4header, void *buffer, unsigned int count)
{

    kernel_multicast(&ipv4protocol.datalinks, buffer, count);

}

static void ipv6protocol_notify(struct ipv6_header *ipv6header, void *buffer, unsigned int count)
{

    kernel_multicast(&ipv6protocol.datalinks, buffer, count);

}

void module_init(void)
{

    ipv4_initprotocol(&ipv4protocol, "icmp", 0x01, ipv4protocol_notify);
    ipv6_initprotocol(&ipv6protocol, "icmp", 0x01, ipv6protocol_notify);

}

void module_register(void)
{

    ipv4_registerprotocol(&ipv4protocol);
    ipv6_registerprotocol(&ipv6protocol);

}

void module_unregister(void)
{

    ipv4_unregisterprotocol(&ipv4protocol);
    ipv6_unregisterprotocol(&ipv6protocol);

}

