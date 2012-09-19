#include <vfs.h>
#include <net/net.h>
#include <ipv4/ipv4.h>

static struct ipv4_protocol protocol;

void init()
{

    ipv4_protocol_init(&protocol);
    net_register_protocol(&protocol.base);

}

void destroy()
{

    net_unregister_protocol(&protocol.base);

}

