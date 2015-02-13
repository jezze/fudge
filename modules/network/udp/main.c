#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <network/ethernet/ethernet.h>
#include <network/ipv4/ipv4.h>

static struct ipv4_protocol ipv4protocol;

void ipv4protocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&ipv4protocol.mailboxes, count, buffer);

}

void init()
{

    ipv4_initprotocol(&ipv4protocol, "udp", 0x11, ipv4protocol_notify);
    ipv4_registerprotocol(&ipv4protocol);

}

void destroy()
{

    ipv4_unregisterprotocol(&ipv4protocol);

}

