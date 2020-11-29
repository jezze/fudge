#include <fudge.h>
#include "ethernet.h"
#include "arp.h"
#include "ipv4.h"
#include "tcp.h"
#include "udp.h"
#include "socket.h"

unsigned short socket_load16(unsigned char seq[2])
{

    return (seq[0] << 8) | (seq[1] << 0);

}

unsigned int socket_load32(unsigned char seq[4])
{

    return (seq[0] << 24) | (seq[1] << 16) | (seq[2] << 8) | (seq[3] << 0);

}

void socket_save16(unsigned char seq[2], unsigned short num)
{

    seq[0] = num >> 8;
    seq[1] = num >> 0;

}

void socket_save32(unsigned char seq[4], unsigned int num)
{

    seq[0] = num >> 24;
    seq[1] = num >> 16;
    seq[2] = num >> 8;
    seq[3] = num >> 0;

}

struct arp_header *socket_arp_create(struct socket *local, struct socket *remote, void *buffer, unsigned short operation)
{

    struct arp_header *header = buffer;

    arp_initheader(header, 1, ETHERNET_ADDRSIZE, ETHERNET_TYPE_IPV4, IPV4_ADDRSIZE, operation);

    return header;

}

struct ethernet_header *socket_ethernet_create(struct socket *local, struct socket *remote, void *buffer)
{

    struct ethernet_header *header = buffer;

    ethernet_initheader(header, ETHERNET_TYPE_IPV4, local->haddress, remote->haddress);

    return header;

}

struct ipv4_header *socket_ipv4_create(struct socket *local, struct socket *remote, void *buffer, unsigned char protocol, unsigned short length)
{

    struct ipv4_header *header = buffer;

    ipv4_initheader(header, local->paddress, remote->paddress, protocol, length);

    return header;

}

struct tcp_header *socket_tcp_create(struct socket *local, struct socket *remote, void *buffer, unsigned short flags, unsigned int seq, unsigned int ack)
{

    struct tcp_header *header = buffer;

    tcp_initheader(header, local->info.tcp.port, remote->info.tcp.port);

    header->flags[0] = (5 << 4);
    header->flags[1] = flags;

    socket_save32(header->seq, seq);
    socket_save32(header->ack, ack);
    socket_save16(header->window, 8192);

    return header;

}

struct udp_header *socket_udp_create(struct socket *local, struct socket *remote, void *buffer, unsigned int count)
{

    struct udp_header *header = buffer;

    udp_initheader(header, local->info.udp.port, remote->info.udp.port, count);

    return header;

}

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

