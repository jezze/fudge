#include <fudge.h>
#include "ipv4.h"
#include "tcp.h"
#include "udp.h"
#include "socket.h"

void socket_init(struct socket *socket, unsigned char protocol, unsigned char address[IPV4_ADDRSIZE])
{

    socket->active = 1;
    socket->protocol = protocol;

    buffer_copy(&socket->address, address, IPV4_ADDRSIZE);

}

void socket_inittcp(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[TCP_PORTSIZE], unsigned int seq)
{

    socket_init(socket, 0x06, address);
    buffer_copy(&socket->body.tcp.port, port, TCP_PORTSIZE);

    socket->body.tcp.state = TCP_STATE_NONE;
    socket->body.tcp.seq = seq;

}

void socket_initudp(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[UDP_PORTSIZE])
{

    socket_init(socket, 0x11, address);
    buffer_copy(&socket->body.udp.port, port, UDP_PORTSIZE);

}

