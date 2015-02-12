#include <fudge.h>
#include <kernel.h>
#include <system/system.h>
#include <base/base.h>
#include <network/network.h>

static struct network_protocol protocol;

void protocol_notify(unsigned int count, void *buffer)
{

    scheduler_mailboxes_send(&protocol.mailboxes, count, buffer);

}

void init()
{

    network_initprotocol(&protocol, "ipv4", 0x0800, protocol_notify);
    network_registerprotocol(&protocol);

}

void destroy()
{

    network_unregisterprotocol(&protocol);

}

