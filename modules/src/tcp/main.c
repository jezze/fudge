#include <vfs.h>
#include <net/net.h>
#include <tcp/tcp.h>

static struct tcp_protocol protocol;

void init()
{

    tcp_protocol_init(&protocol);
    net_register_protocol(&protocol.base);

}

void destroy()
{

    net_unregister_protocol(&protocol.base);

}

