#include <fudge.h>
#include <abi.h>
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

static void send(unsigned int descriptor, void *buffer, unsigned int count)
{

    file_open(descriptor);
    file_writeall(descriptor, buffer, count);
    file_close(descriptor);

}

void socket_loadarplocal(unsigned int descriptor, struct socket *local)
{

    file_open(descriptor);
    file_readall(descriptor, local->haddress, ETHERNET_ADDRSIZE);
    file_close(descriptor);

}

static void savearpremote(struct socket *remote, unsigned char haddress[ETHERNET_ADDRSIZE], unsigned char paddress[IPV4_ADDRSIZE])
{

    buffer_copy(remote->haddress, haddress, ETHERNET_ADDRSIZE);
    buffer_copy(remote->paddress, paddress, IPV4_ADDRSIZE);

}

static unsigned int socket_arp_build(struct socket *local, struct socket *remote, void *output)
{

    unsigned char *data = output;
    struct ethernet_header *eheader = socket_ethernet_create(local, remote, data);
    struct arp_header *aheader = socket_arp_create(local, remote, data + ethernet_hlen(eheader), ARP_REQUEST);

    return ethernet_hlen(eheader) + arp_len(aheader);

}

static unsigned int socket_tcp_build(struct socket *local, struct socket *remote, void *output, unsigned short flags, unsigned int seq, unsigned int ack, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct tcp_header) + count;
    struct ethernet_header *eheader = socket_ethernet_create(local, remote, data);
    struct ipv4_header *iheader = socket_ipv4_create(local, remote, data + ethernet_hlen(eheader), IPV4_PROTOCOL_TCP, length);
    struct tcp_header *theader = socket_tcp_create(local, remote, data + ethernet_hlen(eheader) + ipv4_hlen(iheader), flags, seq, ack);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = tcp_checksum(theader, local->paddress, remote->paddress, tcp_hlen(theader) + count);

    theader->checksum[0] = checksum;
    theader->checksum[1] = checksum >> 8;

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader) + count;

}

static unsigned int socket_udp_build(struct socket *local, struct socket *remote, void *output, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct udp_header) + count;
    struct ethernet_header *eheader = socket_ethernet_create(local, remote, data);
    struct ipv4_header *iheader = socket_ipv4_create(local, remote, data + ethernet_hlen(eheader), IPV4_PROTOCOL_UDP, length);
    struct udp_header *uheader = socket_udp_create(local, remote, data + ethernet_hlen(eheader) + ipv4_hlen(iheader), count);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = udp_checksum(uheader, local->paddress, remote->paddress, udp_hlen(uheader) + count);

    uheader->checksum[0] = checksum;
    uheader->checksum[1] = checksum >> 8;

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader) + count;

}

static unsigned int socket_tcp_receive(unsigned int descriptor, struct socket *local, struct socket *remote, struct tcp_header *header, void *pdata, unsigned int psize)
{

    struct message_data data;

    switch (local->info.tcp.state)
    {

    case TCP_STATE_LISTEN:
        if (header->flags[1] & TCP_FLAGS1_SYN)
        {

            remote->info.tcp.seq = socket_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK | TCP_FLAGS1_SYN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_SYNRECEIVED;

        }

        break;

    case TCP_STATE_SYNSENT:
        if ((header->flags[1] & TCP_FLAGS1_SYN) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            remote->info.tcp.seq = socket_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_ESTABLISHED;

        }

        else if ((header->flags[1] & TCP_FLAGS1_SYN))
        {

            remote->info.tcp.seq = socket_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_SYNRECEIVED;

        }

        break;

    case TCP_STATE_SYNRECEIVED:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            local->info.tcp.seq = socket_load32(header->ack);
            local->info.tcp.state = TCP_STATE_ESTABLISHED;

        }

        break;

    case TCP_STATE_ESTABLISHED:
        if ((header->flags[1] & TCP_FLAGS1_PSH) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            local->info.tcp.seq = socket_load32(header->ack);
            remote->info.tcp.seq = socket_load32(header->seq) + psize;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            return psize;

        }

        else if ((header->flags[1] & TCP_FLAGS1_FIN) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            remote->info.tcp.seq = socket_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_CLOSEWAIT;

            /* Wait for application to close down */

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_FIN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_LASTACK;

        }

        else if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            local->info.tcp.seq = socket_load32(header->ack);

        }

        break;

    case TCP_STATE_FINWAIT1:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_FINWAIT2;

        }

        else if (header->flags[1] & TCP_FLAGS1_FIN)
        {

            remote->info.tcp.seq = socket_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_CLOSING;

        }

        break;

    case TCP_STATE_FINWAIT2:
        if (header->flags[1] & TCP_FLAGS1_FIN)
        {

            local->info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_CLOSEWAIT:
        break;

    case TCP_STATE_CLOSING:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_LASTACK:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_TIMEWAIT:
        break;

    case TCP_STATE_CLOSED:
        break;

    }

    return 0;

}

static unsigned int socket_udp_receive(unsigned int descriptor, struct socket *local, struct socket *remote, struct udp_header *header, void *pdata, unsigned int psize)
{

    return psize;

}

unsigned int socket_receive(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (socket_load16(eheader->type) == ETHERNET_TYPE_ARP)
    {

        struct arp_header *aheader = (struct arp_header *)(data + elen);
        unsigned short alen = arp_hlen(aheader);

        if (socket_load16(aheader->htype) == 0x0001 && socket_load16(aheader->ptype) == ETHERNET_TYPE_IPV4)
        {

            if (socket_load16(aheader->operation) == ARP_REQUEST)
            {

                unsigned char *tip = data + elen + alen + aheader->hlength + aheader->plength + aheader->hlength;

                if (buffer_match(tip, local->paddress, IPV4_ADDRSIZE))
                {

                    unsigned char *sha = data + elen + alen;
                    unsigned char *sip = data + elen + alen + aheader->hlength;

                    savearpremote(remote, sha, sip);

                    /* Send as well */

                }

            }

        }

    }

    else if (socket_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);
            unsigned short tlen = tcp_hlen(theader);

            if (socket_load16(theader->tp) == socket_load16(local->info.tcp.port))
            {

                void *pdata = data + elen + ilen + tlen;
                unsigned int psize = itot - (ilen + tlen);

                if (!remote->active)
                    socket_tcp_init(remote, iheader->sip, theader->sp, socket_load32(theader->seq));

                if (socket_tcp_receive(descriptor, local, remote, theader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

        else if (iheader->protocol == IPV4_PROTOCOL_UDP)
        {

            struct udp_header *uheader = (struct udp_header *)(data + elen + ilen);
            unsigned short ulen = udp_hlen(uheader);

            if (socket_load16(uheader->tp) == socket_load16(local->info.udp.port))
            {

                void *pdata = data + elen + ilen + ulen;
                unsigned int psize = itot - (ilen + ulen);

                if (!remote->active)
                    socket_udp_init(remote, iheader->sip, uheader->sp);

                if (socket_udp_receive(descriptor, local, remote, uheader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

/* add static later */
unsigned int socket_arp_send(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    struct message_data data;

    send(descriptor, &data, socket_arp_build(local, remote, &data));

    return psize;

}

static unsigned int socket_tcp_send(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    struct message_data data;

    switch (local->info.tcp.state)
    {

    case TCP_STATE_ESTABLISHED:
        send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_PSH | TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, psize, pdata));

        return psize;

    }

    return 0;

}

static unsigned int socket_udp_send(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    struct message_data data;

    send(descriptor, &data, socket_udp_build(local, remote, &data, psize, pdata));

    return psize;

}

unsigned int socket_send(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    if (!remote->active)
        return 0;

    switch (local->protocol)
    {

    case IPV4_PROTOCOL_TCP:
        return socket_tcp_send(descriptor, local, remote, psize, pdata);

    case IPV4_PROTOCOL_UDP:
        return socket_udp_send(descriptor, local, remote, psize, pdata);

    }

    return 0;

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

