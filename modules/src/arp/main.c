#include <vfs.h>
#include <net/net.h>
#include <arp/arp.h>

static struct arp_protocol protocol;

void init()
{

    arp_protocol_init(&protocol);
    net_register_protocol(0x0806, &protocol.base);

}

void destroy()
{

    net_unregister_protocol(0x0806);

}

