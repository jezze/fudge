#include <fudge.h>
#include <net.h>
#include <abi.h>

#define TCP_STATE_NONE                  0
#define TCP_STATE_LISTEN                1
#define TCP_STATE_SYNSENT               2
#define TCP_STATE_SYNRECEIVED           3
#define TCP_STATE_ESTABLISHED           4
#define TCP_STATE_FINWAIT1              5
#define TCP_STATE_FINWAIT2              6
#define TCP_STATE_CLOSEWAIT             7
#define TCP_STATE_CLOSING               8
#define TCP_STATE_LASTACK               9
#define TCP_STATE_TIMEWAIT              10
#define TCP_STATE_CLOSED                11

struct endpoint
{

    unsigned int active;
    unsigned int state;
    unsigned int seq;
    unsigned char protocol;
    unsigned char address[IPV4_ADDRSIZE];
    unsigned char port[UDP_PORTSIZE];

};

static struct endpoint local;
static struct endpoint remote;
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

static void endpoint_init(struct endpoint *endpoint)
{

    unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
    unsigned char port[UDP_PORTSIZE] = {0x07, 0xD0};

    endpoint->active = 1;
    endpoint->state = 0;
    endpoint->seq = 42;

    buffer_copy(&endpoint->address, address, IPV4_ADDRSIZE);
    buffer_copy(&endpoint->port, port, UDP_PORTSIZE);

}

static void endpoint_init_udp(struct endpoint *endpoint, struct ipv4_header *iheader, struct udp_header *uheader)
{

    endpoint->active = 1;
    endpoint->state = 0;
    endpoint->seq = 0;
    endpoint->protocol = iheader->protocol;

    buffer_copy(&endpoint->address, iheader->sip, IPV4_ADDRSIZE);
    buffer_copy(&endpoint->port, uheader->sp, UDP_PORTSIZE);

}

static void endpoint_init_tcp(struct endpoint *endpoint, struct ipv4_header *iheader, struct tcp_header *theader)
{

    endpoint->active = 1;
    endpoint->state = TCP_STATE_NONE;
    endpoint->seq = loadint(theader->seq);
    endpoint->protocol = iheader->protocol;

    buffer_copy(&endpoint->address, iheader->sip, IPV4_ADDRSIZE);
    buffer_copy(&endpoint->port, theader->sp, UDP_PORTSIZE);

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

            if (loadshort(eheader->type) == 0x0800)
            {

                struct ipv4_header *iheader = (struct ipv4_header *)(eheader + 1);

                if (iheader->protocol == 0x11)
                {

                    struct udp_header *uheader = (struct udp_header *)(iheader + 1);

                    if (loadshort(uheader->tp) == loadshort(local.port))
                    {

                        unsigned int length = loadshort(uheader->length) - sizeof (struct udp_header);

                        if (!remote.active)
                            endpoint_init_udp(&remote, iheader, uheader);

                        channel_place(channel, source, EVENT_DATA, length, uheader + 1);

                    }

                }

                else if (iheader->protocol == 0x06)
                {

                    struct tcp_header *theader = (struct tcp_header *)(iheader + 1);

                    if (loadshort(theader->tp) == loadshort(local.port))
                    {

                        if (!remote.active)
                            endpoint_init_tcp(&remote, iheader, theader);

                        if (theader->flags[1] & TCP_FLAGS1_SYN)
                        {

                            struct ipv4_arpentry *sentry = findarpentry(local.address);
                            struct ipv4_arpentry *tentry = findarpentry(remote.address);
                            struct ethernet_header eheader2;
                            struct ipv4_header iheader2;
                            struct {struct tcp_header theader2; char data[2];} xxx;
                            unsigned int msgsize = sizeof (struct tcp_header);
                            unsigned int offset = 0;

                            xxx.data[0] = 'H';
                            xxx.data[1] = 'i';

                            if (sentry && tentry)
                            {

                                unsigned short checksum;

                                local.state = TCP_STATE_SYNRECEIVED;
                                remote.seq = loadint(theader->seq);

                                channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP SYN received! Sending SYN+ACK\n", 0), &data);

                                ethernet_initheader(&eheader2, 0x0800, sentry->haddress, tentry->haddress);

                                ipv4_initheader(&iheader2, local.address, remote.address, 0x06, msgsize);
                                tcp_initheader(&xxx.theader2, local.port, remote.port);

                                saveint(xxx.theader2.seq, local.seq);
                                saveint(xxx.theader2.ack, remote.seq + 1);

                                xxx.theader2.flags[0] = (5 << 4);
                                xxx.theader2.flags[1] = TCP_FLAGS1_ACK | TCP_FLAGS1_SYN;

                                saveshort(xxx.theader2.window, 8192);

                                checksum = tcp_checksum(iheader2.sip, iheader2.tip, msgsize, (unsigned short *)&xxx);

                                xxx.theader2.checksum[0] = checksum;
                                xxx.theader2.checksum[1] = checksum >> 8;

                                offset = message_putbuffer(&data, sizeof (struct ethernet_header), &eheader2, offset);
                                offset = message_putbuffer(&data, sizeof (struct ipv4_header), &iheader2, offset);
                                offset = message_putbuffer(&data, sizeof (struct tcp_header), &xxx.theader2, offset);

                                file_open(FILE_G0);
                                file_writeall(FILE_G0, data.buffer, offset);
                                file_close(FILE_G0);

                            }

                        }

                        else if (theader->flags[1] & TCP_FLAGS1_ACK)
                        {

                            local.state = TCP_STATE_ESTABLISHED;

                            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP ACK received! Communication established\n", 0), &data);

                        }

                        else if (theader->flags[1] & TCP_FLAGS1_FIN)
                        {

                            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP FIN received! Sending ACK+FIN\n", 0), &data);

                        }

                        else
                        {

                            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP received!\n", 0), &data);

                        }

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

                struct ipv4_arpentry *sentry = findarpentry(local.address);
                struct ipv4_arpentry *tentry = findarpentry(remote.address);

                if (sentry && tentry)
                {

                    if (remote.protocol == 0x11)
                    {

                        struct ethernet_header eheader;
                        struct ipv4_header iheader;
                        struct udp_header uheader;
                        unsigned int offset = 0;

                        ethernet_initheader(&eheader, 0x0800, sentry->haddress, tentry->haddress);
                        ipv4_initheader(&iheader, local.address, remote.address, 0x11, sizeof (struct udp_header) + length);
                        udp_initheader(&uheader, local.port, remote.port, length);

                        offset = message_putbuffer(&data, sizeof (struct ethernet_header), &eheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct ipv4_header), &iheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct udp_header), &uheader, offset);
                        offset = message_putbuffer(&data, length, &s, offset);

                        file_open(FILE_G0);
                        file_writeall(FILE_G0, data.buffer, offset);
                        file_close(FILE_G0);

                    }

                    else if (remote.protocol == 0x06)
                    {
/*
                        struct ethernet_header eheader;
                        struct ipv4_header iheader;
                        struct tcp_header theader;
                        unsigned int offset = 0;

                        ethernet_initheader(&eheader, 0x0800, sentry->haddress, tentry->haddress);
                        ipv4_initheader(&iheader, local.address, remote.address, 0x11, sizeof (struct tcp_header) + length);
                        tcp_initheader(&theader, local.port, remote.port);

                        offset = message_putbuffer(&data, sizeof (struct ethernet_header), &eheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct ipv4_header), &iheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct tcpp_header), &theader, offset);
                        offset = message_putbuffer(&data, length, &s, offset);

                        file_open(FILE_G0);
                        file_writeall(FILE_G0, data.buffer, offset);
                        file_close(FILE_G0);
*/

                        channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP send!\n", 0), &data);

                    }

                }

            }

        }

    }

    file_unlink(FILE_G0);
    channel_close(channel, source);

}

void init(struct channel *channel)
{

    endpoint_init(&local);

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    if (!file_walk2(FILE_G2, "/system/ethernet/ipv4/arptable"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

