#include <fudge.h>
#include <abi.h>
#include <net.h>
#include "socket.h"

static void send(unsigned int descriptor, void *buffer, unsigned int count)
{

    file_open(descriptor);
    file_writeall(descriptor, buffer, count);
    file_close(descriptor);

}

static struct arp_header *createarp(struct socket *local, struct socket *remote, void *buffer, unsigned short operation)
{

    struct arp_header *header = buffer;

    arp_initheader(header, 1, ETHERNET_ADDRSIZE, ETHERNET_TYPE_IPV4, IPV4_ADDRSIZE, operation);

    return header;

}

static struct ethernet_header *createethernet(struct socket *local, struct socket *remote, unsigned short type, void *buffer)
{

    struct ethernet_header *header = buffer;

    ethernet_initheader(header, type, local->haddress, remote->haddress);

    return header;

}

static struct ethernet_header *createethernet2(struct socket *local, struct socket *remote, unsigned short type, void *buffer)
{

    struct ethernet_header *header = buffer;
    unsigned char tha[ETHERNET_ADDRSIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    ethernet_initheader(header, type, local->haddress, tha);

    return header;

}

static struct ipv4_header *createipv4(struct socket *local, struct socket *remote, void *buffer, unsigned char protocol, unsigned short length)
{

    struct ipv4_header *header = buffer;

    ipv4_initheader(header, local->paddress, remote->paddress, protocol, length);

    return header;

}

static struct icmp_header *createicmp(struct socket *local, struct socket *remote, void *buffer, unsigned char type, unsigned char code)
{

    struct icmp_header *header = buffer;

    icmp_initheader(header, type, code);

    return header;

}

static struct tcp_header *createtcp(struct socket *local, struct socket *remote, void *buffer, unsigned short flags, unsigned int seq, unsigned int ack)
{

    struct tcp_header *header = buffer;

    tcp_initheader(header, local->info.tcp.port, remote->info.tcp.port);

    header->flags[0] = (5 << 4);
    header->flags[1] = flags;

    net_save32(header->seq, seq);
    net_save32(header->ack, ack);
    net_save16(header->window, 8192);

    return header;

}

static struct udp_header *createudp(struct socket *local, struct socket *remote, void *buffer, unsigned int count)
{

    struct udp_header *header = buffer;

    udp_initheader(header, local->info.udp.port, remote->info.udp.port, count);

    return header;

}

unsigned int socket_arp_build(struct socket *local, struct socket *remote, void *output, unsigned short operation, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char sip[IPV4_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE])
{

    unsigned char *data = output;
    struct ethernet_header *eheader = createethernet(local, remote, ETHERNET_TYPE_ARP, data);
    struct arp_header *aheader = createarp(local, remote, data + ethernet_hlen(eheader), operation);
    void *psha = (data + ethernet_hlen(eheader) + arp_hlen(aheader));
    void *psip = (data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE);
    void *ptha = (data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE);
    void *ptip = (data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE);

    buffer_copy(psha, sha, ETHERNET_ADDRSIZE); 
    buffer_copy(psip, sip, IPV4_ADDRSIZE); 
    buffer_copy(ptha, tha, ETHERNET_ADDRSIZE); 
    buffer_copy(ptip, tip, IPV4_ADDRSIZE); 

    return ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE;

}

unsigned int socket_arp_build2(struct socket *local, struct socket *remote, void *output, unsigned short operation, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char sip[IPV4_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE])
{

    unsigned char *data = output;
    struct ethernet_header *eheader = createethernet2(local, remote, ETHERNET_TYPE_ARP, data);
    struct arp_header *aheader = createarp(local, remote, data + ethernet_hlen(eheader), operation);
    void *psha = (data + ethernet_hlen(eheader) + arp_hlen(aheader));
    void *psip = (data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE);
    void *ptha = (data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE);
    void *ptip = (data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE);

    buffer_copy(psha, sha, ETHERNET_ADDRSIZE); 
    buffer_copy(psip, sip, IPV4_ADDRSIZE); 
    buffer_copy(ptha, tha, ETHERNET_ADDRSIZE); 
    buffer_copy(ptip, tip, IPV4_ADDRSIZE); 

    return ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE;

}

unsigned int socket_icmp_build(struct socket *local, struct socket *remote, void *output, unsigned char type, unsigned char code, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct icmp_header) + count;
    struct ethernet_header *eheader = createethernet(local, remote, ETHERNET_TYPE_IPV4, data);
    struct ipv4_header *iheader = createipv4(local, remote, data + ethernet_hlen(eheader), IPV4_PROTOCOL_ICMP, length);
    struct icmp_header *icmpheader = createicmp(local, remote, data + ethernet_hlen(eheader) + ipv4_hlen(iheader), type, code);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + icmp_hlen(icmpheader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = icmp_calculatechecksum(icmpheader, icmp_hlen(icmpheader) + count);

    buffer_copy(icmpheader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + icmp_hlen(icmpheader) + count;

}

unsigned int socket_tcp_build(struct socket *local, struct socket *remote, void *output, unsigned short flags, unsigned int seq, unsigned int ack, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct tcp_header) + count;
    struct ethernet_header *eheader = createethernet(local, remote, ETHERNET_TYPE_IPV4, data);
    struct ipv4_header *iheader = createipv4(local, remote, data + ethernet_hlen(eheader), IPV4_PROTOCOL_TCP, length);
    struct tcp_header *theader = createtcp(local, remote, data + ethernet_hlen(eheader) + ipv4_hlen(iheader), flags, seq, ack);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = tcp_checksum(theader, local->paddress, remote->paddress, tcp_hlen(theader) + count);

    buffer_copy(theader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader) + count;

}

unsigned int socket_udp_build(struct socket *local, struct socket *remote, void *output, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct udp_header) + count;
    struct ethernet_header *eheader = createethernet(local, remote, ETHERNET_TYPE_IPV4, data);
    struct ipv4_header *iheader = createipv4(local, remote, data + ethernet_hlen(eheader), IPV4_PROTOCOL_UDP, length);
    struct udp_header *uheader = createudp(local, remote, data + ethernet_hlen(eheader) + ipv4_hlen(iheader), count);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = udp_checksum(uheader, local->paddress, remote->paddress, udp_hlen(uheader) + count);

    buffer_copy(uheader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader) + count;

}

unsigned int socket_ethernet_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);

    return buffer_write(output, outputcount, eheader, ethernet_hlen(eheader), 0);
    
}

unsigned int socket_arp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_ARP)
    {

        struct arp_header *aheader = (struct arp_header *)(data + elen);

        return buffer_write(output, outputcount, aheader, arp_len(aheader), 0);

    }

    return 0;

}

unsigned int socket_ipv4_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);

        return buffer_write(output, outputcount, iheader, ipv4_len(iheader), 0);

    }

    return 0;

}

unsigned int socket_icmp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct icmp_header *icmpheader = (struct icmp_header *)(data + elen + ilen);

            return buffer_write(output, outputcount, icmpheader, itot, 0);

        }

    }

    return 0;

}

unsigned int socket_tcp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);

            return buffer_write(output, outputcount, theader, itot, 0);

        }

    }

    return 0;

}

unsigned int socket_udp_read(unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_UDP)
        {

            struct tcp_header *uheader = (struct tcp_header *)(data + elen + ilen);

            return buffer_write(output, outputcount, uheader, itot, 0);

        }

    }

    return 0;

}

unsigned int socket_icmp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct icmp_header *header, void *pdata, unsigned int psize)
{

    struct message_data data;

    switch (header->type)
    {

    case ICMP_ECHOREQUEST:
        send(descriptor, &data, socket_icmp_build(local, remote, &data, ICMP_ECHOREPLY, 0, psize, pdata));

        break;

    }

    return 0;

}

unsigned int socket_tcp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct tcp_header *header, void *pdata, unsigned int psize)
{

    struct message_data data;

    switch (local->info.tcp.state)
    {

    case TCP_STATE_LISTEN:
        if (header->flags[1] == TCP_FLAGS1_SYN)
        {

            local->info.tcp.state = TCP_STATE_SYNRECEIVED;
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK | TCP_FLAGS1_SYN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        break;

    case TCP_STATE_SYNSENT:
        if (header->flags[1] == (TCP_FLAGS1_SYN | TCP_FLAGS1_ACK))
        {

            local->info.tcp.state = TCP_STATE_ESTABLISHED;
            local->info.tcp.seq = net_load32(header->ack);
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        else if (header->flags[1] == TCP_FLAGS1_SYN)
        {

            local->info.tcp.state = TCP_STATE_SYNRECEIVED;
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        break;

    case TCP_STATE_SYNRECEIVED:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_ESTABLISHED;
            local->info.tcp.seq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_ESTABLISHED:
        if (header->flags[1] == (TCP_FLAGS1_PSH | TCP_FLAGS1_ACK))
        {

            local->info.tcp.seq = net_load32(header->ack);
            remote->info.tcp.seq = net_load32(header->seq) + psize;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            return psize;

        }

        else if (header->flags[1] == (TCP_FLAGS1_FIN | TCP_FLAGS1_ACK))
        {

            local->info.tcp.state = TCP_STATE_CLOSEWAIT;
            local->info.tcp.seq = net_load32(header->ack);
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            /* Wait for application to close down */

            local->info.tcp.state = TCP_STATE_LASTACK;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_FIN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        else if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.seq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_FINWAIT1:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_FINWAIT2;
            local->info.tcp.seq = net_load32(header->ack);

        }

        else if (header->flags[1] == TCP_FLAGS1_FIN)
        {

            local->info.tcp.state = TCP_STATE_CLOSING;
            remote->info.tcp.seq = net_load32(header->seq) + 1;

            send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

        }

        break;

    case TCP_STATE_FINWAIT2:
        if (header->flags[1] == TCP_FLAGS1_FIN)
        {

            local->info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_CLOSEWAIT:
        break;

    case TCP_STATE_CLOSING:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_TIMEWAIT;
            local->info.tcp.seq = net_load32(header->ack);

            /* Sleep some time */

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_LASTACK:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            local->info.tcp.state = TCP_STATE_CLOSED;
            local->info.tcp.seq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_TIMEWAIT:
        break;

    case TCP_STATE_CLOSED:
        break;

    }

    return 0;

}

unsigned int socket_udp_respond(unsigned int descriptor, struct socket *local, struct socket *remote, struct udp_header *header, void *pdata, unsigned int psize)
{

    return psize;

}

unsigned int socket_receive(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_ICMP)
        {

            struct icmp_header *icmpheader = (struct icmp_header *)(data + elen + ilen);
            unsigned short icmplen = icmp_hlen(icmpheader);
            void *pdata = data + elen + ilen + icmplen;
            unsigned int psize = itot - (ilen + icmplen);

            socket_icmp_respond(descriptor, local, remote, icmpheader, pdata, psize);

        }

        else if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);
            unsigned short tlen = tcp_hlen(theader);

            if (net_load16(theader->tp) == net_load16(local->info.tcp.port))
            {

                void *pdata = data + elen + ilen + tlen;
                unsigned int psize = itot - (ilen + tlen);

#if 0
                if (!remote->resolved)
                    socket_tcp_init(remote, iheader->sip, theader->sp, net_load32(theader->seq));
#endif

                if (socket_tcp_respond(descriptor, local, remote, theader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

        else if (iheader->protocol == IPV4_PROTOCOL_UDP)
        {

            struct udp_header *uheader = (struct udp_header *)(data + elen + ilen);
            unsigned short ulen = udp_hlen(uheader);

            if (net_load16(uheader->tp) == net_load16(local->info.udp.port))
            {

                void *pdata = data + elen + ilen + ulen;
                unsigned int psize = itot - (ilen + ulen);

#if 0
                if (!remote->resolved)
                    socket_udp_init(remote, iheader->sip, uheader->sp);
#endif

                if (socket_udp_respond(descriptor, local, remote, uheader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

static unsigned int sendtcp(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
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

static unsigned int sendudp(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    struct message_data data;

    send(descriptor, &data, socket_udp_build(local, remote, &data, psize, pdata));

    return psize;

}

unsigned int socket_send(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    switch (protocol)
    {

    case IPV4_PROTOCOL_TCP:
        return sendtcp(descriptor, local, remote, psize, pdata);

    case IPV4_PROTOCOL_UDP:
        return sendudp(descriptor, local, remote, psize, pdata);

    }

    return 0;

}

void socket_listen(unsigned int descriptor, unsigned char protocol, struct socket *local)
{

    switch (protocol)
    {

    case IPV4_PROTOCOL_TCP:
        local->info.tcp.state = TCP_STATE_LISTEN;

        break;

    case IPV4_PROTOCOL_UDP:
        break;

    }

}

void socket_connect(unsigned int descriptor, unsigned char protocol, struct socket *local, struct socket *remote)
{

    struct message_data data;

    switch (protocol)
    {

    case IPV4_PROTOCOL_TCP:
        local->info.tcp.state = TCP_STATE_SYNSENT;

        send(descriptor, &data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_SYN, local->info.tcp.seq, 0, 0, 0));

        break;

    case IPV4_PROTOCOL_UDP:
        break;

    }

}

void socket_resolveremote(unsigned int descriptor, struct socket *local, struct socket *remote)
{

    struct message_data data;

    send(descriptor, &data, socket_arp_build2(local, remote, &data, ARP_REQUEST, local->haddress, local->paddress, remote->haddress, remote->paddress));

}

void socket_resolvelocal(unsigned int descriptor, struct socket *socket)
{

    file_open(descriptor);
    file_readall(descriptor, socket->haddress, ETHERNET_ADDRSIZE);
    file_close(descriptor);

    socket->resolved = 1;

}

void socket_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE])
{

    socket->resolved = 0;

    buffer_copy(&socket->paddress, address, IPV4_ADDRSIZE);

}

void socket_tcp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[TCP_PORTSIZE], unsigned int seq)
{

    socket_init(socket, address);
    buffer_copy(&socket->info.tcp.port, port, TCP_PORTSIZE);

    socket->info.tcp.state = TCP_STATE_CLOSED;
    socket->info.tcp.seq = seq;

}

void socket_udp_init(struct socket *socket, unsigned char address[IPV4_ADDRSIZE], unsigned char port[UDP_PORTSIZE])
{

    socket_init(socket, address);
    buffer_copy(&socket->info.udp.port, port, UDP_PORTSIZE);

}

