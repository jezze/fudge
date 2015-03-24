#include <fudge.h>
#include <net/ipv6.h>
#include <kernel.h>
#include <system/system.h>
#include <network/ethernet/ethernet.h>

static struct ethernet_protocol ethernetprotocol;

static void ethernetprotocol_addinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_removeinterface(struct ethernet_interface *interface)
{

}

static void ethernetprotocol_notify(struct ethernet_interface *interface, unsigned int count, void *buffer)
{

    scheduler_sendlist(&ethernetprotocol.data.mailboxes, count, buffer);

}

void module_init()
{

    ethernet_initprotocol(&ethernetprotocol, "ipv6", 0x86DD, ethernetprotocol_addinterface, ethernetprotocol_removeinterface, ethernetprotocol_notify);

}

void module_register()
{

    ethernet_registerprotocol(&ethernetprotocol);

}

void module_unregister()
{

    ethernet_unregisterprotocol(&ethernetprotocol);

}

