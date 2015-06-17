#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>
#include <modules/ethernet/ethernet.h>
#include <modules/ipv4/ipv4.h>

static struct ipv4_protocol ipv4protocol;

void ipv4protocol_notify(struct ethernet_interface *interface, unsigned int size, unsigned int count, void *buffer)
{

    rendezvous_writelist(&ipv4protocol.data.mailboxes, size, count, buffer);

}

void module_init()
{

    ipv4_initprotocol(&ipv4protocol, "udp", 0x11, ipv4protocol_notify);

}

void module_register()
{

    ipv4_registerprotocol(&ipv4protocol);

}

void module_unregister()
{

    ipv4_unregisterprotocol(&ipv4protocol);

}

