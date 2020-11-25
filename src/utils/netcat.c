#include <fudge.h>
#include <net.h>
#include <abi.h>

static struct socket local;
static struct socket remote;
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

static unsigned int create_tcp_message(struct message_data *data, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned short flags, unsigned int seq, unsigned int ack, unsigned int count, void *buffer)
{

    struct ethernet_header *eheader = (struct ethernet_header *)data;
    struct ipv4_header *iheader = (struct ipv4_header *)(eheader + 1);
    struct tcp_header *theader = (struct tcp_header *)(iheader + 1);
    void *payload = (void *)(theader + 1);
    unsigned int length = sizeof (struct tcp_header) + count;
    unsigned short checksum;

    ethernet_initheader(eheader, ETHERNET_TYPE_IPV4, sha, tha);
    ipv4_initheader(iheader, local.address, remote.address, IPV4_PROTOCOL_TCP, length);
    tcp_initheader(theader, local.info.tcp.port, remote.info.tcp.port);
    buffer_copy(payload, buffer, count); 

    theader->flags[0] = (5 << 4);
    theader->flags[1] = flags;

    saveint(theader->seq, seq);
    saveint(theader->ack, ack);
    saveshort(theader->window, 8192);

    checksum = tcp_checksum(iheader->sip, iheader->tip, length, (unsigned short *)theader);

    theader->checksum[0] = checksum;
    theader->checksum[1] = checksum >> 8;

    return sizeof (struct ethernet_header) + sizeof (struct ipv4_header) + length;

}

static unsigned int create_udp_message(struct message_data *data, unsigned char sha[ETHERNET_ADDRSIZE], unsigned char tha[ETHERNET_ADDRSIZE], unsigned int count, void *buffer)
{

    struct ethernet_header *eheader = (struct ethernet_header *)data;
    struct ipv4_header *iheader = (struct ipv4_header *)(eheader + 1);
    struct udp_header *uheader = (struct udp_header *)(iheader + 1);
    void *payload = (void *)(uheader + 1);
    unsigned int length = sizeof (struct udp_header) + count;

    ethernet_initheader(eheader, ETHERNET_TYPE_IPV4, sha, tha);
    ipv4_initheader(iheader, local.address, remote.address, IPV4_PROTOCOL_UDP, length);
    udp_initheader(uheader, local.info.udp.port, remote.info.udp.port, count);
    buffer_copy(payload, buffer, count); 

    /* Create checksum */

    return sizeof (struct ethernet_header) + sizeof (struct ipv4_header) + length;

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

static void tcp_listen(void)
{

    local.info.tcp.state = TCP_STATE_LISTEN;

}

#if 0
static void tcp_connect(void)
{

    /* Send SYN */

    local.info.tcp.state = TCP_STATE_SYNSENT;

}
#endif

static void handle_tcp_receive(struct channel *channel, unsigned int source, struct tcp_header *header, void *payload, unsigned int psize)
{

    struct ipv4_arpentry *sentry = findarpentry(local.address);
    struct ipv4_arpentry *tentry = findarpentry(remote.address);
    struct message_data data;

    if (!sentry || !tentry)
        return;

    switch (local.info.tcp.state)
    {

    case TCP_STATE_LISTEN:
        if (header->flags[1] & TCP_FLAGS1_SYN)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[SYN : SYN+ACK] LISTEN -> SYNRECEIVED\n", 0), &data);

            remote.info.tcp.seq = loadint(header->seq) + 1;

            file_open(FILE_G0);
            file_writeall(FILE_G0, &data, create_tcp_message(&data, sentry->haddress, tentry->haddress, TCP_FLAGS1_ACK | TCP_FLAGS1_SYN, local.info.tcp.seq, remote.info.tcp.seq, 0, 0));
            file_close(FILE_G0);

            local.info.tcp.state = TCP_STATE_SYNRECEIVED;

        }

        break;

    case TCP_STATE_SYNSENT:
        if ((header->flags[1] & TCP_FLAGS1_SYN) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[SYN+ACK : ACK] SYNSENT -> ESTABLISHED\n", 0), &data);

            /* SEND ACK */

            local.info.tcp.state = TCP_STATE_ESTABLISHED;

        }

        break;

    case TCP_STATE_SYNRECEIVED:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] SYNRECEIVED -> ESTABLISHED\n", 0), &data);

            local.info.tcp.seq = loadint(header->ack);
            local.info.tcp.state = TCP_STATE_ESTABLISHED;

        }

        break;

    case TCP_STATE_ESTABLISHED:
        if ((header->flags[1] & TCP_FLAGS1_PSH) && (header->flags[1] & TCP_FLAGS1_ACK))
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[PSH+ACK : ACK] ESTABLISHED -> ESTABLISHED\n", 0), &data);

            local.info.tcp.seq = loadint(header->ack);
            remote.info.tcp.seq = loadint(header->seq) + psize;

            channel_place(channel, source, EVENT_DATA, message_putstring(&data, payload, psize), &data);

            file_open(FILE_G0);
            file_writeall(FILE_G0, &data, create_tcp_message(&data, sentry->haddress, tentry->haddress, TCP_FLAGS1_ACK, local.info.tcp.seq, remote.info.tcp.seq, 0, 0));
            file_close(FILE_G0);

        }

        else if (header->flags[1] & TCP_FLAGS1_FIN)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[FIN : ACK] ESTABLISHED -> CLOSEWAIT\n", 0), &data);

            /* SEND ACK */

            local.info.tcp.state = TCP_STATE_CLOSEWAIT;

            /* Wait for application to close down */

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[___ : FIN] CLOSEWAIT -> LASTACK\n", 0), &data);

            /* SEND FIN */

            local.info.tcp.state = TCP_STATE_LASTACK;

        }

        break;

    case TCP_STATE_FINWAIT1:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] FINWAIT1 -> FINWAIT2\n", 0), &data);

            local.info.tcp.state = TCP_STATE_FINWAIT2;

        }

        else if (header->flags[1] & TCP_FLAGS1_FIN)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[FIN : ACK] FINWAIT1 -> CLOSING\n", 0), &data);

            /* Send ACK */

            local.info.tcp.state = TCP_STATE_CLOSING;

        }

        break;

    case TCP_STATE_FINWAIT2:
        if (header->flags[1] & TCP_FLAGS1_FIN)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[FIN : ACK] FINWAIT2 -> TIMEWAIT\n", 0), &data);

            local.info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[___ : ___] TIMEWAIT -> CLOSED\n", 0), &data);

            local.info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_CLOSEWAIT:
        break;

    case TCP_STATE_CLOSING:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] CLOSING -> TIMEWAIT\n", 0), &data);

            local.info.tcp.state = TCP_STATE_TIMEWAIT;

            /* Sleep some time */

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[___ : ___] TIMEWAIT -> CLOSED\n", 0), &data);

            local.info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_LASTACK:
        if (header->flags[1] & TCP_FLAGS1_ACK)
        {

            /* remove later */
            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "[ACK : ___] LASTACK -> CLOSED\n", 0), &data);

            local.info.tcp.state = TCP_STATE_CLOSED;

        }

        break;

    case TCP_STATE_TIMEWAIT:
        break;

    case TCP_STATE_CLOSED:
        break;

    }

}

static void handle_tcp_send(struct channel *channel, unsigned int source, unsigned int length, void *buffer)
{

    struct message_data data;

    channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP send!\n", 0), &data);

}

static void handle_udp_receive(struct channel *channel, unsigned int source, struct udp_header *header, void *payload, unsigned int psize)
{

    channel_place(channel, source, EVENT_DATA, psize, payload);

}

static void handle_udp_send(struct channel *channel, unsigned int source, unsigned int length, void *buffer)
{

    struct ipv4_arpentry *sentry = findarpentry(local.address);
    struct ipv4_arpentry *tentry = findarpentry(remote.address);
    struct message_data data;

    if (!sentry || !tentry)
        return;

    file_open(FILE_G0);
    file_writeall(FILE_G0, &data, create_udp_message(&data, sentry->haddress, tentry->haddress, length, buffer));
    file_close(FILE_G0);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);

    while (channel_poll(channel, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            struct ethernet_header *eheader = (struct ethernet_header *)data.buffer;

            if (loadshort(eheader->type) == ETHERNET_TYPE_IPV4)
            {

                struct ipv4_header *iheader = (struct ipv4_header *)(eheader + 1);

                if (iheader->protocol == IPV4_PROTOCOL_TCP)
                {

                    struct tcp_header *theader = (struct tcp_header *)(iheader + 1);

                    if (loadshort(theader->tp) == loadshort(local.info.tcp.port))
                    {

                        if (!remote.active)
                            socket_inittcp(&remote, iheader->sip, theader->sp, loadint(theader->seq));

                        handle_tcp_receive(channel, source, theader, theader + 1, loadshort(iheader->length) - sizeof (struct ipv4_header) - sizeof (struct tcp_header));

                    }

                }

                else if (iheader->protocol == IPV4_PROTOCOL_UDP)
                {

                    struct udp_header *uheader = (struct udp_header *)(iheader + 1);

                    if (loadshort(uheader->tp) == loadshort(local.info.udp.port))
                    {

                        if (!remote.active)
                            socket_initudp(&remote, iheader->sip, uheader->sp);

                        handle_udp_receive(channel, source, uheader, uheader + 1, loadshort(iheader->length) - sizeof (struct ipv4_header) - sizeof (struct udp_header));

                    }

                }

            }

        }

        if (header.event == EVENT_CONSOLEDATA)
        {

            struct event_consoledata *consoledata = (struct event_consoledata *)data.buffer;
            char s = consoledata->data;
            unsigned int length = 1;

            channel_place(channel, source, EVENT_DATA, length, &s);

            if (remote.active)
            {

                switch (remote.protocol)
                {

                case IPV4_PROTOCOL_TCP:
                    handle_tcp_send(channel, source, length, &s);

                    break;

                case IPV4_PROTOCOL_UDP:
                    handle_udp_send(channel, source, length, &s);

                    break;

                }

            }

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port[UDP_PORTSIZE] = {0x07, 0xD0};

    socket_inittcp(&local, address, port, 42);
    tcp_listen();

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    if (!file_walk2(FILE_G2, "/system/ethernet/ipv4/arptable"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

