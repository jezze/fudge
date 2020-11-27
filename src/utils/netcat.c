#include <fudge.h>
#include <net.h>
#include <abi.h>

static struct ipv4_arpentry arptable[8];

static unsigned short loadshort(unsigned char seq[2])
{

    return (seq[0] << 8) | (seq[1] << 0);

}

static unsigned int loadint(unsigned char seq[4])
{

    return (seq[0] << 24) | (seq[1] << 16) | (seq[2] << 8) | (seq[3] << 0);

}

static void saveshort(unsigned char seq[2], unsigned short num)
{

    seq[0] = num >> 8;
    seq[1] = num >> 0;

}

static void saveint(unsigned char seq[4], unsigned int num)
{

    seq[0] = num >> 24;
    seq[1] = num >> 16;
    seq[2] = num >> 8;
    seq[3] = num >> 0;

}

static void send(void *buffer, unsigned int count)
{

    file_open(FILE_G0);
    file_writeall(FILE_G0, buffer, count);
    file_close(FILE_G0);

}

static struct ipv4_arpentry *findarpentry(void *paddress)
{

    unsigned int i;

    file_open(FILE_G2);
    file_readall(FILE_G2, arptable, sizeof (struct ipv4_arpentry) * 8);
    file_close(FILE_G2);

    for (i = 0; i < 8; i++)
    {

        if (buffer_match(arptable[i].paddress, paddress, IPV4_ADDRSIZE))
            return &arptable[i];

    }

    return 0;

}

static struct ethernet_header *socket_ethernet_create(struct socket *local, struct socket *remote, void *buffer)
{

    struct ethernet_header *header = buffer;
    struct ipv4_arpentry *sentry = findarpentry(local->address);
    struct ipv4_arpentry *tentry = findarpentry(remote->address);

    if (!sentry || !tentry)
        return 0;

    ethernet_initheader(header, ETHERNET_TYPE_IPV4, sentry->haddress, tentry->haddress);

    return header;

}

static struct ipv4_header *socket_ipv4_create(struct socket *local, struct socket *remote, void *buffer, unsigned char protocol, unsigned short length)
{

    struct ipv4_header *header = buffer;

    ipv4_initheader(header, local->address, remote->address, protocol, length);

    return header;

}

static struct tcp_header *socket_tcp_create(struct socket *local, struct socket *remote, void *buffer, unsigned short flags, unsigned int seq, unsigned int ack)
{

    struct tcp_header *header = buffer;

    tcp_initheader(header, local->info.tcp.port, remote->info.tcp.port);

    header->flags[0] = (5 << 4);
    header->flags[1] = flags;

    saveint(header->seq, seq);
    saveint(header->ack, ack);
    saveshort(header->window, 8192);

    return header;

}

static struct udp_header *socket_udp_create(struct socket *local, struct socket *remote, void *buffer, unsigned int count)
{

    struct udp_header *header = buffer;

    udp_initheader(header, local->info.udp.port, remote->info.udp.port, count);

    return header;

}

static unsigned int socket_tcp_build(struct socket *local, struct socket *remote, struct message_data *data, unsigned short flags, unsigned int seq, unsigned int ack, unsigned int count, void *buffer)
{

    unsigned int length = sizeof (struct tcp_header) + count;
    struct ethernet_header *eheader = socket_ethernet_create(local, remote, data);
    struct ipv4_header *iheader = socket_ipv4_create(local, remote, (char *)eheader + ethernet_hlen(eheader), IPV4_PROTOCOL_TCP, length);
    struct tcp_header *theader = socket_tcp_create(local, remote, (char *)iheader + ipv4_hlen(iheader), flags, seq, ack);
    void *pdata = ((char *)theader + tcp_hlen(theader));
    unsigned short checksum;

    buffer_copy(pdata, buffer, count); 

    checksum = tcp_checksum(theader, local->address, remote->address, tcp_hlen(theader) + count);

    theader->checksum[0] = checksum;
    theader->checksum[1] = checksum >> 8;

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + tcp_hlen(theader) + count;

}

static unsigned int socket_udp_build(struct socket *local, struct socket *remote, struct message_data *data, unsigned int count, void *buffer)
{

    unsigned int length = sizeof (struct udp_header) + count;
    struct ethernet_header *eheader = socket_ethernet_create(local, remote, data);
    struct ipv4_header *iheader = socket_ipv4_create(local, remote, (char *)eheader + ethernet_hlen(eheader), IPV4_PROTOCOL_UDP, length);
    struct udp_header *uheader = socket_udp_create(local, remote, (char *)iheader + ipv4_hlen(iheader), count);
    void *pdata = ((char *)uheader + udp_hlen(uheader));

    buffer_copy(pdata, buffer, count); 

    /* Create checksum */

    return ethernet_hlen(eheader) + ipv4_hlen(iheader) + udp_hlen(uheader) + count;

}

static unsigned int socket_tcp_receive(struct socket *local, struct socket *remote, struct tcp_header *header, void *pdata, unsigned int psize, struct channel *channel, unsigned int source)
{

    struct message_data data;

    switch (local->info.tcp.state)
    {

    case TCP_STATE_LISTEN:
        if (header->flags[1] & TCP_FLAGS1_SYN)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[SYN : SYN+ACK] LISTEN -> SYNRECEIVED\n", 0), &data);

            remote->info.tcp.seq = loadint(header->seq) + 1;

            send(&data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK | TCP_FLAGS1_SYN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_SYNRECEIVED;

        }

        break;

    case TCP_STATE_SYNSENT:
        if ((header->flags[1] & TCP_FLAGS1_SYN) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[SYN+ACK : ACK] SYNSENT -> ESTABLISHED\n", 0), &data);

            /* SEND ACK */

            local->info.tcp.state = TCP_STATE_ESTABLISHED;

        }

        break;

    case TCP_STATE_SYNRECEIVED:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] SYNRECEIVED -> ESTABLISHED\n", 0), &data);

            local->info.tcp.seq = loadint(header->ack);
            local->info.tcp.state = TCP_STATE_ESTABLISHED;

        }

        break;

    case TCP_STATE_ESTABLISHED:
        if ((header->flags[1] & TCP_FLAGS1_PSH) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            local->info.tcp.seq = loadint(header->ack);
            remote->info.tcp.seq = loadint(header->seq) + psize;

            send(&data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            return psize;

        }

        else if ((header->flags[1] & TCP_FLAGS1_FIN) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[FIN+ACK : ACK] ESTABLISHED -> CLOSEWAIT\n", 0), &data);

            remote->info.tcp.seq = loadint(header->seq) + 1;

            send(&data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_CLOSEWAIT;

            /* Wait for application to close down */

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[___ : FIN] CLOSEWAIT -> LASTACK\n", 0), &data);

            send(&data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_FIN, local->info.tcp.seq, remote->info.tcp.seq, 0, 0));

            local->info.tcp.state = TCP_STATE_LASTACK;

        }

        else if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            local->info.tcp.seq = loadint(header->ack);

        }

        break;

    case TCP_STATE_FINWAIT1:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] FINWAIT1 -> FINWAIT2\n", 0), &data);

            local->info.tcp.state = TCP_STATE_FINWAIT2;

        }

        else if (header->flags[1] & TCP_FLAGS1_FIN)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[FIN : ACK] FINWAIT1 -> CLOSING\n", 0), &data);

            /* Send ACK */

            local->info.tcp.state = TCP_STATE_CLOSING;

        }

        break;

    case TCP_STATE_FINWAIT2:
        if (header->flags[1] & TCP_FLAGS1_FIN)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[FIN : ACK] FINWAIT2 -> TIMEWAIT\n", 0), &data);

            local->info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[___ : ___] TIMEWAIT -> CLOSED\n", 0), &data);

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_CLOSEWAIT:
        break;

    case TCP_STATE_CLOSING:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] CLOSING -> TIMEWAIT\n", 0), &data);

            local->info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[___ : ___] TIMEWAIT -> CLOSED\n", 0), &data);

            local->info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_LASTACK:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] LASTACK -> CLOSED\n", 0), &data);

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

static unsigned int socket_udp_receive(struct socket *local, struct socket *remote, struct udp_header *header, void *pdata, unsigned int psize)
{

    return psize;

}

static unsigned int socket_receive(struct socket *local, struct socket *remote, unsigned int count, void *buffer, unsigned int outputcount, void *output, struct channel *channel, unsigned int source)
{

    unsigned char *data = buffer;
    struct ethernet_header *eheader = (struct ethernet_header *)(data);
    unsigned short elen = ethernet_hlen(eheader);

    if (loadshort(eheader->type) == ETHERNET_TYPE_IPV4)
    {

        struct ipv4_header *iheader = (struct ipv4_header *)(data + elen);
        unsigned short ilen = ipv4_hlen(iheader);
        unsigned short itot = ipv4_len(iheader);

        if (iheader->protocol == IPV4_PROTOCOL_TCP)
        {

            struct tcp_header *theader = (struct tcp_header *)(data + elen + ilen);
            unsigned short tlen = tcp_hlen(theader);

            if (loadshort(theader->tp) == loadshort(local->info.tcp.port))
            {

                void *pdata = data + elen + ilen + tlen;
                unsigned int psize = itot - (ilen + tlen);

                if (!remote->active)
                    socket_tcp_init(remote, iheader->sip, theader->sp, loadint(theader->seq));

                if (socket_tcp_receive(local, remote, theader, pdata, psize, channel, source))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

        else if (iheader->protocol == IPV4_PROTOCOL_UDP)
        {

            struct udp_header *uheader = (struct udp_header *)(data + elen + ilen);
            unsigned short ulen = udp_hlen(uheader);

            if (loadshort(uheader->tp) == loadshort(local->info.udp.port))
            {

                void *pdata = data + elen + ilen + ulen;
                unsigned int psize = itot - (ilen + ulen);

                if (!remote->active)
                    socket_udp_init(remote, iheader->sip, uheader->sp);

                if (socket_udp_receive(local, remote, uheader, pdata, psize))
                    return buffer_write(output, outputcount, pdata, psize, 0);

            }

        }

    }

    return 0;

}

static unsigned int socket_tcp_send(struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    struct message_data data;

    switch (local->info.tcp.state)
    {

    case TCP_STATE_ESTABLISHED:
        send(&data, socket_tcp_build(local, remote, &data, TCP_FLAGS1_PSH | TCP_FLAGS1_ACK, local->info.tcp.seq, remote->info.tcp.seq, psize, pdata));

        return psize;

    }

    return 0;

}

static unsigned int socket_udp_send(struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    struct message_data data;

    send(&data, socket_udp_build(local, remote, &data, psize, pdata));

    return psize;

}

static unsigned int socket_send(struct socket *local, struct socket *remote, unsigned int psize, void *pdata)
{

    if (!remote->active)
        return 0;

    switch (local->protocol)
    {

    case IPV4_PROTOCOL_TCP:
        return socket_tcp_send(local, remote, psize, pdata);

    case IPV4_PROTOCOL_UDP:
        return socket_udp_send(local, remote, psize, pdata);

    }

    return 0;

}

static struct socket local;
static struct socket remote;

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);

    while (channel_pollsource(channel, 0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            char buffer[BUFFER_SIZE];
            unsigned int count = socket_receive(&local, &remote, message_datasize(&header), &data, BUFFER_SIZE, &buffer, channel, source);

            if (count)
                channel_place(channel, source, EVENT_DATA, count, buffer);

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

static void onconsoledata(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    unsigned int count = 0;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\t':
        break;

    case '\b':
    case 0x7F:
        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        count = socket_send(&local, &remote, 1, &consoledata->data);

        break;

    default:
        count = socket_send(&local, &remote, 1, &consoledata->data);

        break;

    }

    if (count)
        channel_place(channel, source, EVENT_DATA, count, &consoledata->data);

}

void init(struct channel *channel)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port[UDP_PORTSIZE] = {0x07, 0xD0};

    socket_tcp_init(&local, address, port, 42);
    socket_listen(&local);

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    if (!file_walk2(FILE_G2, "/system/ethernet/ipv4/arptable"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_CONSOLEDATA, onconsoledata);

}

