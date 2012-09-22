#include <vfs.h>
#include <net/net.h>
#include <ipv4/ipv4.h>

static struct ipv4_protocol protocol;

void init()
{

    ipv4_init_protocol(&protocol);
    net_register_protocol(0x0800, &protocol.base);

}

void destroy()
{

    net_unregister_protocol(0x0800);

}

