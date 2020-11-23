#include <fudge.h>
#include <net.h>
#include <abi.h>

struct remote
{

    unsigned int active;
    unsigned int state;
    unsigned char protocol;
    unsigned char address[IPV4_ADDRSIZE];
    unsigned char port[UDP_PORTSIZE];

};

static struct remote incoming;
static struct ipv4_arpentry arptable[8];
static unsigned char address[IPV4_ADDRSIZE] = {10, 0, 5, 1};
static unsigned char port[UDP_PORTSIZE] = {0x07, 0xD0};

static void remote_init_udp(struct remote *remote, struct ipv4_header *iheader, struct udp_header *uheader)
{

    remote->active = 1;
    remote->state = 0;
    remote->protocol = iheader->protocol;

    buffer_copy(&remote->address, iheader->sip, IPV4_ADDRSIZE);
    buffer_copy(&remote->port, uheader->sp, UDP_PORTSIZE);

}

static void remote_init_tcp(struct remote *remote, struct ipv4_header *iheader, struct tcp_header *theader)
{

    remote->active = 1;
    remote->protocol = iheader->protocol;

    buffer_copy(&remote->address, iheader->sip, IPV4_ADDRSIZE);
    buffer_copy(&remote->port, theader->sp, UDP_PORTSIZE);

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

            if (eheader->type[0] == 0x08 && eheader->type[1] == 0x00)
            {

                struct ipv4_header *iheader = (struct ipv4_header *)(eheader + 1);

                if (iheader->protocol == 0x11)
                {

                    struct udp_header *uheader = (struct udp_header *)(iheader + 1);

                    if (uheader->tp[0] == port[0] && uheader->tp[1] == port[1])
                    {

                        char *payload = (char *)(uheader + 1);
                        unsigned int length = (uheader->length[1] | uheader->length[0] << 8) - sizeof (struct udp_header);

                        if (!incoming.active)
                            remote_init_udp(&incoming, iheader, uheader);

                        channel_place(channel, source, EVENT_DATA, length, payload);

                    }

                }

                else if (iheader->protocol == 0x06)
                {

                    struct tcp_header *theader = (struct tcp_header *)(iheader + 1);

                    if (theader->tp[0] == port[0] && theader->tp[1] == port[1])
                    {

                        if (!incoming.active)
                            remote_init_tcp(&incoming, iheader, theader);

                        if (theader->flags[1] && TCP_FLAGS1_SYN)
                        {

                            incoming.state = 1;

                            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP SYN received! Sending SYN+ACK\n", 0), &data);

                            /* SEND SYN + ACK */

                        }

                        else if (theader->flags[0] && TCP_FLAGS0_ACK)
                        {

                            incoming.state = 2;

                            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP ACK received! Communication established\n", 0), &data);

                        }

                        else if (theader->flags[1] && TCP_FLAGS1_FIN)
                        {

                            incoming.state = 4;

                            channel_place(channel, source, EVENT_DATA, message_putstring(&data, "TCP FIN received!\n", 0), &data);

                        }

                        else
                        {

                            incoming.state = 3;

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

            if (incoming.active)
            {

                struct ipv4_arpentry *sentry = findarpentry(address);
                struct ipv4_arpentry *tentry = findarpentry(incoming.address);

                if (sentry && tentry)
                {

                    if (incoming.protocol == 0x11)
                    {

                        struct ethernet_header eheader;
                        struct ipv4_header iheader;
                        struct udp_header uheader;
                        unsigned int offset = 0;

                        ethernet_initheader(&eheader, 0x0800, sentry->haddress, tentry->haddress);
                        ipv4_initheader(&iheader, address, incoming.address, 0x11, sizeof (struct udp_header) + length);
                        udp_initheader(&uheader, port, incoming.port, length);

                        offset = message_putbuffer(&data, sizeof (struct ethernet_header), &eheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct ipv4_header), &iheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct udp_header), &uheader, offset);
                        offset = message_putbuffer(&data, length, &s, offset);

                        file_open(FILE_G0);
                        file_writeall(FILE_G0, data.buffer, offset);
                        file_close(FILE_G0);

                    }

                    else if (incoming.protocol == 0x06)
                    {
/*
                        struct ethernet_header eheader;
                        struct ipv4_header iheader;
                        struct tcp_header theader;
                        unsigned int offset = 0;

                        ethernet_initheader(&eheader, 0x0800, sentry->haddress, tentry->haddress);
                        ipv4_initheader(&iheader, address, incoming.address, 0x11, sizeof (struct udp_header) + length);
                        tcp_initheader(&theader, port, incoming.port);

                        offset = message_putbuffer(&data, sizeof (struct ethernet_header), &eheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct ipv4_header), &iheader, offset);
                        offset = message_putbuffer(&data, sizeof (struct udp_header), &uheader, offset);
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

    if (!file_walk2(FILE_G0, "/system/ethernet/if:0/data"))
        return;

    if (!file_walk2(FILE_G1, "/system/ethernet/if:0/addr"))
        return;

    if (!file_walk2(FILE_G2, "/system/ethernet/ipv4/arptable"))
        return;

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

