#include <fudge.h>
#include "ethernet.h"
#include "arp.h"
#include "ipv4.h"
#include "tcp.h"
#include "udp.h"
#include "socket.h"

void socket_listen(struct socket *local)
{

    switch (local->protocol)
    {

    case IPV4_PROTOCOL_TCP:
        local->info.tcp.state = TCP_STATE_LISTEN;

        break;

    case IPV4_PROTOCOL_UDP:
        break;

    }

}

void socket_connect(struct socket *local, struct socket *remote)
{

    switch (local->protocol)
    {

    case IPV4_PROTOCOL_TCP:
        /* TODO: Send SYN */

        local->info.tcp.state = TCP_STATE_SYNSENT;

        break;

    case IPV4_PROTOCOL_UDP:
        break;

    }

}

void socket_init(struct socket *socket, unsigned char protocol, unsigned char address[IPV4_ADDRSIZE])
{

    socket->active = 1;
    socket->protocol = protocol;

    buffer_copy(&socket->paddress, address, IPV4_ADDRSIZE);

}

void socket_tcp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[TCP_PORTSIZE], unsigned int seq)
{

    socket_init(socket, 0x06, address);
    buffer_copy(&socket->info.tcp.port, port, TCP_PORTSIZE);

    socket->info.tcp.state = TCP_STATE_CLOSED;
    socket->info.tcp.seq = seq;

}

void socket_udp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[UDP_PORTSIZE])
{

    socket_init(socket, 0x11, address);
    buffer_copy(&socket->info.udp.port, port, UDP_PORTSIZE);

}

