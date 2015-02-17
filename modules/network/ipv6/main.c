#include <fudge.h>
#include <net/ipv6.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <network/ethernet/ethernet.h>

static struct ethernet_protocol protocol;

void protocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&protocol.mailboxes, count, buffer);

}

void module_init()
{

    ethernet_initprotocol(&protocol, "ipv6", 0x86DD, protocol_notify);

}

void module_register()
{

    ethernet_registerprotocol(&protocol);

}

void module_unregister()
{

    ethernet_unregisterprotocol(&protocol);

}

