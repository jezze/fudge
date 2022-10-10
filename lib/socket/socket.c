#include <fudge.h>
#include <abi.h>
#include <net.h>
#include "socket.h"

static unsigned int convert(unsigned char address[IPV4_ADDRSIZE], char *buffer)
{

    unsigned int i;
    unsigned int start = 0;
    unsigned int n = 0;

    for (i = 0; i < cstring_lengthz(buffer); i++)
    {

        if (buffer[i] == '.' || buffer[i] == '\0')
        {

            address[n] = cstring_readvalue(buffer + start, i - start, 10);

            start = i + 1;
            n++;

        }

    }

    return n;

}

static void send(unsigned int descriptor, void *buffer, unsigned int count)
{

    file_seekwriteall(descriptor, buffer, count, 0);

}

static unsigned int buildarp(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned short operation, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char sip[IPV4_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned char tip[IPV4_ADDRSIZE])
{

    unsigned char *data = output;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_ARP, local->haddress, router->haddress);
    struct arp_header *aheader = arp_putheader(data + ethernet_hlen(eheader), 1, ETHERNET_ADDRSIZE, ETHERNET_TYPE_IPV4, IPV4_ADDRSIZE, operation);

    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader), sha, ETHERNET_ADDRSIZE); 
    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE, sip, IPV4_ADDRSIZE); 
    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE, tha, ETHERNET_ADDRSIZE); 
    buffer_copy(data + ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE, tip, IPV4_ADDRSIZE); 

    return ethernet_hlen(eheader) + arp_hlen(aheader) + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE + ETHERNET_ADDRSIZE + IPV4_ADDRSIZE;

}

static unsigned int buildicmp(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned char type, unsigned char code, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct icmp_header) + count;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_IPV4, local->haddress, router->haddress);
    struct ipv4_header *iheader = ipv4_putheader(data + ethernet_hlen(eheader), local->paddress, remote->paddress, IPV4_PROTOCOL_ICMP, length);
    struct icmp_header *icmpheader = icmp_putheader(data + ethernet_hlen(eheader) + ipv4_hlen(iheader), type, code);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + icmp_hlen(icmpheader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = icmp_calculatechecksum(icmpheader, icmp_hlen(icmpheader) + count);

    buffer_copy(icmpheader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + icmp_hlen(icmpheader) + count;

}

static unsigned int buildtcp(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned short flags, unsigned int seq, unsigned int ack, unsigned int window, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct tcp_header) + count;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_IPV4, local->haddress, router->haddress);
    struct ipv4_header *iheader = ipv4_putheader(data + ethernet_hlen(eheader), local->paddress, remote->paddress, IPV4_PROTOCOL_TCP, length);
    struct tcp_header *theader = tcp_putheader(data + ethernet_hlen(eheader) + ipv4_hlen(iheader), local->info.tcp.port, remote->info.tcp.port, flags, seq, ack, window);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = tcp_checksum(theader, local->paddress, remote->paddress, tcp_hlen(theader) + count);

    buffer_copy(theader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader) + count;

}

static unsigned int buildudp(struct socket *local, struct socket *remote, struct socket *router, void *output, unsigned int count, void *buffer)
{

    unsigned char *data = output;
    unsigned int length = sizeof (struct udp_header) + count;
    struct ethernet_header *eheader = ethernet_putheader(data, ETHERNET_TYPE_IPV4, local->haddress, router->haddress);
    struct ipv4_header *iheader = ipv4_putheader(data + ethernet_hlen(eheader), local->paddress, remote->paddress, IPV4_PROTOCOL_UDP, length);
    struct udp_header *uheader = udp_putheader(data + ethernet_hlen(eheader) + ipv4_hlen(iheader), local->info.udp.port, remote->info.udp.port, count);
    void *pdata = (data + ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = udp_checksum(uheader, local->paddress, remote->paddress, udp_hlen(uheader) + count);

    buffer_copy(uheader->checksum, &checksum, 2);

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader) + count;

}

static unsigned int respondarp(unsigned int descriptor, struct socket *local, struct socket *remote, struct arp_header *header, unsigned char *pdata, unsigned int psize)
{

    switch (net_load16(header->operation))
    {

    case ARP_REQUEST:
        if (buffer_match(pdata + net_load8(header->hlength) + net_load8(header->plength) + net_load8(header->hlength), local->paddress, IPV4_ADDRSIZE))
        {

            struct message_data data;
            struct socket answer;

            socket_init(&answer);
            buffer_copy(answer.haddress, pdata, ETHERNET_ADDRSIZE);
            buffer_copy(answer.paddress, pdata + net_load8(header->hlength), IPV4_ADDRSIZE);

            send(descriptor, data.buffer, buildarp(local, &answer, &answer, data.buffer, ARP_REPLY, local->haddress, local->paddress, pdata, pdata + net_load8(header->hlength)));

        }

        break;

    case ARP_REPLY:
        buffer_copy(remote->haddress, pdata, ETHERNET_ADDRSIZE);

        remote->resolved = 1;

        break;

    }

    return 0;

}

static unsigned int respondicmp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct icmp_header *header, unsigned char *pdata, unsigned int psize)
{

    struct message_data data;

    switch (net_load8(header->type))
    {

    case ICMP_ECHOREQUEST:
        send(descriptor, data.buffer, buildicmp(local, remote, router, data.buffer, ICMP_ECHOREPLY, 0, psize, pdata));

        break;

    }

    return 0;

}

static unsigned int respondtcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct tcp_header *header, unsigned char *pdata, unsigned int psize)
{

    struct message_data data;

    switch (remote->info.tcp.state)
    {

    case TCP_STATE_LISTEN:
        if (header->flags[1] == TCP_FLAGS1_SYN)
        {

            remote->info.tcp.state = TCP_STATE_SYNRECEIVED;
            remote->info.tcp.remoteseq = net_load32(header->seq) + 1;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_ACK | TCP_FLAGS1_SYN, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

        }

        break;

    case TCP_STATE_SYNSENT:
        if (header->flags[1] == (TCP_FLAGS1_SYN | TCP_FLAGS1_ACK))
        {

            remote->info.tcp.state = TCP_STATE_ESTABLISHED;
            remote->info.tcp.localseq = net_load32(header->ack);
            remote->info.tcp.remoteseq = net_load32(header->seq) + 1;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_ACK, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

        }

        else if (header->flags[1] == TCP_FLAGS1_SYN)
        {

            remote->info.tcp.state = TCP_STATE_SYNRECEIVED;
            remote->info.tcp.remoteseq = net_load32(header->seq) + 1;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_ACK, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

        }

        break;

    case TCP_STATE_SYNRECEIVED:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            remote->info.tcp.state = TCP_STATE_ESTABLISHED;
            remote->info.tcp.localseq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_ESTABLISHED:
        if (header->flags[1] == (TCP_FLAGS1_PSH | TCP_FLAGS1_ACK))
        {

            remote->info.tcp.localseq = net_load32(header->ack);
            remote->info.tcp.remoteseq = net_load32(header->seq) + psize;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_ACK, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

            return psize;

        }

        else if (header->flags[1] == (TCP_FLAGS1_FIN | TCP_FLAGS1_PSH | TCP_FLAGS1_ACK))
        {

            remote->info.tcp.state = TCP_STATE_CLOSEWAIT;
            remote->info.tcp.localseq = net_load32(header->ack);
            remote->info.tcp.remoteseq = net_load32(header->seq) + psize + 1;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_ACK, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

            /* Wait for application to close down */

            remote->info.tcp.state = TCP_STATE_LASTACK;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_FIN, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

            return psize;

        }

        else if (header->flags[1] == (TCP_FLAGS1_FIN | TCP_FLAGS1_ACK))
        {

            remote->info.tcp.state = TCP_STATE_CLOSEWAIT;
            remote->info.tcp.localseq = net_load32(header->ack);
            remote->info.tcp.remoteseq = net_load32(header->seq) + 1;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_ACK, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

            /* Wait for application to close down */

            remote->info.tcp.state = TCP_STATE_LASTACK;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_FIN, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

        }

        else if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            remote->info.tcp.localseq = net_load32(header->ack);

            return psize;

        }

        break;

    case TCP_STATE_FINWAIT1:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            remote->info.tcp.state = TCP_STATE_FINWAIT2;
            remote->info.tcp.localseq = net_load32(header->ack);

        }

        else if (header->flags[1] == TCP_FLAGS1_FIN)
        {

            remote->info.tcp.state = TCP_STATE_CLOSING;
            remote->info.tcp.remoteseq = net_load32(header->seq) + 1;

            send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_ACK, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, 0, 0));

        }

        break;

    case TCP_STATE_FINWAIT2:
        if (header->flags[1] == TCP_FLAGS1_FIN)
        {

            remote->info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            remote->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_CLOSEWAIT:
        break;

    case TCP_STATE_CLOSING:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            remote->info.tcp.state = TCP_STATE_TIMEWAIT;
            remote->info.tcp.localseq = net_load32(header->ack);

            /* Sleep some time */

            remote->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_LASTACK:
        if (header->flags[1] == TCP_FLAGS1_ACK)
        {

            remote->info.tcp.state = TCP_STATE_CLOSED;
            remote->info.tcp.localseq = net_load32(header->ack);

        }

        break;

    case TCP_STATE_TIMEWAIT:
        break;

    case TCP_STATE_CLOSED:
        break;

    }

    return 0;

}

static unsigned int respondudp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, struct udp_header *header, unsigned char *pdata, unsigned int psize)
{

    return psize;

}

unsigned int socket_handle_arp(unsigned int descriptor, struct socket *local, struct socket *remote, unsigned int count, void *buffer)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_ARP)
    {

        struct arp_header *aheader = (struct arp_header *)(data + elen);
        unsigned short alen = arp_hlen(aheader);
        void *pdata = data + elen + alen;
        unsigned int psize = 0; /* figure this out */

        respondarp(descriptor, local, remote, aheader, pdata, psize);

    }

    return 0;

}

unsigned int socket_handle_icmp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (net_load8(iheader->protocol) == IPV4_PROTOCOL_ICMP)
        {

            struct icmp_header *icmpheader = (struct icmp_header *)(data + elen + ilen);
            unsigned short icmplen = icmp_hlen(icmpheader);
            void *pdata = data + elen + ilen + icmplen;
            unsigned int psize = itot - (ilen + icmplen);

            respondicmp(descriptor, local, remote, router, icmpheader, pdata, psize);

        }

    }

    return 0;

}

unsigned int socket_handle_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (net_load8(iheader->protocol) == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);
            unsigned short tlen = tcp_hlen(theader);

            if (net_load16(theader->tp) == net_load16(local->info.tcp.port))
            {

                void *pdata = data + elen + ilen + tlen;
                unsigned int psize = itot - (ilen + tlen);

                if (respondtcp(descriptor, local, remote, router, theader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

unsigned int socket_handle_udp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int count, void *buffer, unsigned int outputcount, void *output)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (net_load8(iheader->protocol) == IPV4_PROTOCOL_UDP)
        {

            struct udp_header *uheader = (struct udp_header *)(data + elen + ilen);
            unsigned short ulen = udp_hlen(uheader);

            if (net_load16(uheader->tp) == net_load16(local->info.udp.port))
            {

                void *pdata = data + elen + ilen + ulen;
                unsigned int psize = itot - (ilen + ulen);

                if (respondudp(descriptor, local, remote, router, uheader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

unsigned int socket_send_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata)
{

    struct message_data data;

    switch (remote->info.tcp.state)
    {

    case TCP_STATE_ESTABLISHED:
        send(descriptor, data.buffer, buildtcp(local, remote, router, data.buffer, TCP_FLAGS1_PSH | TCP_FLAGS1_ACK, remote->info.tcp.localseq, remote->info.tcp.remoteseq, BUFFER_SIZE, psize, pdata));

        return psize;

    }

    return 0;

}

unsigned int socket_send_udp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router, unsigned int psize, void *pdata)
{

    struct message_data data;

    send(descriptor, data.buffer, buildudp(local, remote, router, data.buffer, psize, pdata));

    return psize;

}

static struct socket *acceptarp(struct socket *local, struct socket *remotes, unsigned int nremotes, unsigned int count, void *buffer)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);

    if (net_load16(eheader->type) == ETHERNET_TYPE_ARP)
    {

        return &remotes[0];

    }

    return 0;

}

static struct socket *accepttcp(struct socket *local, struct socket *remotes, unsigned int nremotes, unsigned int count, void *buffer)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);

        if (net_load8(iheader->protocol) == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);

            if (net_load16(theader->tp) == net_load16(local->info.tcp.port))
            {

                unsigned int i;

                for (i = 0; i < nremotes; i++)
                {

                    struct socket *remote = &remotes[i];

                    if (remote->resolved)
                    {

                        if (!buffer_match(remote->haddress, eheader->sha, ETHERNET_ADDRSIZE))
                            continue;

                        if (!buffer_match(remote->paddress, iheader->sip, IPV4_ADDRSIZE))
                            continue;

                        if (!buffer_match(remote->info.tcp.port, theader->sp, TCP_PORTSIZE))
                            continue;

                        return remote;

                    }

                    else
                    {

                        buffer_copy(remote->haddress, eheader->sha, ETHERNET_ADDRSIZE);
                        buffer_copy(remote->paddress, iheader->sip, IPV4_ADDRSIZE);
                        buffer_copy(remote->info.tcp.port, theader->sp, TCP_PORTSIZE);

                        remote->info.tcp.remoteseq = net_load32(theader->seq);
                        remote->resolved = 1;

                        return remote;

                    }

                }

            }

        }

    }

    return 0;

}

static struct socket *acceptudp(struct socket *local, struct socket *remotes, unsigned int nremotes, unsigned int count, void *buffer)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (net_load16(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);

        if (net_load8(iheader->protocol) == IPV4_PROTOCOL_UDP)
        {

            struct udp_header *uheader = (struct udp_header *)(data + elen + ilen);

            if (net_load16(uheader->tp) == net_load16(local->info.udp.port))
            {

                unsigned int i;

                for (i = 0; i < nremotes; i++)
                {

                    struct socket *remote = &remotes[i];

                    if (remote->resolved)
                    {

                        if (!buffer_match(remote->haddress, eheader->sha, ETHERNET_ADDRSIZE))
                            continue;

                        if (!buffer_match(remote->paddress, iheader->sip, IPV4_ADDRSIZE))
                            continue;

                        if (!buffer_match(remote->info.udp.port, uheader->sp, UDP_PORTSIZE))
                            continue;

                        return remote;

                    }

                    else
                    {

                        buffer_copy(remote->haddress, eheader->sha, ETHERNET_ADDRSIZE);
                        buffer_copy(remote->paddress, iheader->sip, IPV4_ADDRSIZE);
                        buffer_copy(remote->info.udp.port, uheader->sp, UDP_PORTSIZE);

                        remote->resolved = 1;

                        return remote;

                    }

                }

            }

        }

    }

    return 0;

}

unsigned int socket_receive(unsigned int descriptor, struct socket *local, struct socket *remotes, unsigned int nremotes, struct socket *router, void *buffer, unsigned int count)
{

    struct message message;

    while (channel_kpollevent(EVENT_DATA, &message))
    {

        struct socket *remote;

        remote = acceptarp(local, remotes, nremotes, message_datasize(&message.header), message.data.buffer);

        if (remote)
        {

            socket_handle_arp(descriptor, local, remote, message_datasize(&message.header), message.data.buffer);

        }

        remote = accepttcp(local, remotes, nremotes, message_datasize(&message.header), message.data.buffer);

        if (remote)
        {

            unsigned int payploadcount;

            if (remote->info.tcp.state != TCP_STATE_ESTABLISHED)
                return 0;

            payploadcount = socket_handle_tcp(descriptor, local, remote, router, message_datasize(&message.header), message.data.buffer, BUFFER_SIZE, buffer);

            if (payploadcount)
                return payploadcount;

            if (remote->info.tcp.state != TCP_STATE_ESTABLISHED)
                return 0;

        }

        remote = acceptudp(local, remotes, nremotes, message_datasize(&message.header), message.data.buffer);

        if (remote)
        {

            unsigned int payploadcount = socket_handle_udp(descriptor, local, remote, router, message_datasize(&message.header), message.data.buffer, BUFFER_SIZE, buffer);

            if (payploadcount)
                return payploadcount;

        }

    }

    return 0;

}

void socket_listen_tcp(unsigned int descriptor, struct socket *local, struct socket *remotes, unsigned int nremotes, struct socket *router)
{

    struct message message;
    unsigned int i;

    for (i = 0; i < nremotes; i++)
        remotes[i].info.tcp.state = TCP_STATE_LISTEN;

    while (channel_kpollevent(EVENT_DATA, &message))
    {

        struct socket *remote;

        remote = acceptarp(local, remotes, nremotes, message_datasize(&message.header), message.data.buffer);

        if (remote)
        {

            socket_handle_arp(descriptor, local, remote, message_datasize(&message.header), message.data.buffer);

        }

        remote = accepttcp(local, remotes, nremotes, message_datasize(&message.header), message.data.buffer);

        if (remote)
        {

            char buffer[BUFFER_SIZE];

            socket_handle_tcp(descriptor, local, remote, router, message_datasize(&message.header), message.data.buffer, BUFFER_SIZE, buffer);

            if (remote->info.tcp.state == TCP_STATE_ESTABLISHED)
                return;

        }

    }

}

void socket_connect_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router)
{

    struct message message;

    remote->info.tcp.state = TCP_STATE_SYNSENT;

    send(descriptor, message.data.buffer, buildtcp(local, remote, router, message.data.buffer, TCP_FLAGS1_SYN, remote->info.tcp.localseq, 0, BUFFER_SIZE, 0, 0));

    while (channel_kpollevent(EVENT_DATA, &message))
    {

        char buffer[BUFFER_SIZE];

        socket_handle_arp(descriptor, local, remote, message_datasize(&message.header), message.data.buffer);
        socket_handle_tcp(descriptor, local, remote, router, message_datasize(&message.header), message.data.buffer, BUFFER_SIZE, buffer);

        if (remote->info.tcp.state == TCP_STATE_ESTABLISHED)
            break;

    }

}

void socket_disconnect_tcp(unsigned int descriptor, struct socket *local, struct socket *remote, struct socket *router)
{

    struct message message;

    while (channel_kpollevent(EVENT_DATA, &message))
    {

        char buffer[BUFFER_SIZE];

        socket_handle_arp(descriptor, local, remote, message_datasize(&message.header), message.data.buffer);
        socket_handle_tcp(descriptor, local, remote, router, message_datasize(&message.header), message.data.buffer, BUFFER_SIZE, buffer);

        /* No timeout implemented */
        if (remote->info.tcp.state == TCP_STATE_TIMEWAIT)
            break;

        /* Sometimes no last ack is sent back */
        if (remote->info.tcp.state == TCP_STATE_LASTACK)
            break;

        if (remote->info.tcp.state == TCP_STATE_CLOSED)
            break;

    }

}

void socket_resolveremote(unsigned int descriptor, struct socket *local, struct socket *remote)
{

    struct socket multicast;
    struct message message;
    unsigned char haddress[ETHERNET_ADDRSIZE] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    buffer_copy(&multicast, remote, sizeof (struct socket));
    buffer_copy(multicast.haddress, haddress, ETHERNET_ADDRSIZE); 
    send(descriptor, message.data.buffer, buildarp(local, remote, &multicast, message.data.buffer, ARP_REQUEST, local->haddress, local->paddress, remote->haddress, remote->paddress));

    while (channel_kpollevent(EVENT_DATA, &message))
    {

        socket_handle_arp(descriptor, local, remote, message_datasize(&message.header), message.data.buffer);

        if (remote->resolved)
            break;

    }

}

void socket_resolvelocal(unsigned int descriptor, struct socket *socket)
{

    file_seekreadall(descriptor, socket->haddress, ETHERNET_ADDRSIZE, 0);

    socket->resolved = 1;

}

void socket_bind_ipv4(struct socket *socket, unsigned char address[IPV4_ADDRSIZE])
{

    buffer_copy(&socket->paddress, address, IPV4_ADDRSIZE);

}

void socket_bind_ipv4s(struct socket *socket, char *address)
{

    unsigned char a[IPV4_ADDRSIZE];

    if (convert(a, address) == IPV4_ADDRSIZE)
        socket_bind_ipv4(socket, a);

}

void socket_bind_tcp(struct socket *socket, unsigned char port[TCP_PORTSIZE], unsigned int localseq, unsigned int remoteseq)
{

    buffer_copy(&socket->info.tcp.port, port, TCP_PORTSIZE);

    socket->info.tcp.state = TCP_STATE_CLOSED;
    socket->info.tcp.localseq = localseq;
    socket->info.tcp.remoteseq = remoteseq;

}

void socket_bind_tcps(struct socket *socket, char *port, unsigned int localseq, unsigned int remoteseq)
{

    unsigned char p[TCP_PORTSIZE];

    net_save16(p, cstring_readvalue(port, cstring_length(port), 10));
    socket_bind_tcp(socket, p, localseq, remoteseq);
 
}

void socket_bind_tcpv(struct socket *socket, unsigned short port, unsigned int localseq, unsigned int remoteseq)
{

    unsigned char p[TCP_PORTSIZE];

    net_save16(p, port);
    socket_bind_tcp(socket, p, localseq, remoteseq);
 
}

void socket_bind_udp(struct socket *socket, unsigned char port[UDP_PORTSIZE])
{

    buffer_copy(&socket->info.udp.port, port, UDP_PORTSIZE);

}

void socket_bind_udps(struct socket *socket, char *port)
{

    unsigned char p[UDP_PORTSIZE];

    net_save16(p, cstring_readvalue(port, cstring_length(port), 10));
    socket_bind_udp(socket, p);
 
}

void socket_bind_udpv(struct socket *socket, unsigned short port)
{

    unsigned char p[UDP_PORTSIZE];

    net_save16(p, port);
    socket_bind_udp(socket, p);
 
}

void socket_init(struct socket *socket)
{

    socket->resolved = 0;

}

