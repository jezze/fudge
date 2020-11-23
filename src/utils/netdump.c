#include <fudge.h>
#include <net.h>
#include <abi.h>

static void print_ethernet(struct channel *channel, unsigned int source, struct ethernet_header *header)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "Ethernet:\n", offset);
    offset = message_putstring(&data, "  Source Addr (MAC): ", offset);
    offset = message_putvalue(&data, header->sha[0], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sha[1], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sha[2], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sha[3], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sha[4], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sha[5], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Target Addr (MAC): ", offset);
    offset = message_putvalue(&data, header->tha[0], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tha[1], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tha[2], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tha[3], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tha[4], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tha[5], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Type: 0x", offset);
    offset = message_putvalue(&data, header->type[0], 16, 2, offset);
    offset = message_putvalue(&data, header->type[1], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);

}

static void print_arp(struct channel *channel, unsigned int source, struct arp_header *header)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "ARP:\n", offset);
    offset = message_putstring(&data, "  Hardware Type: 0x", offset);
    offset = message_putvalue(&data, header->htype[0], 16, 2, offset);
    offset = message_putvalue(&data, header->htype[1], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Protocol Type: 0x", offset);
    offset = message_putvalue(&data, header->ptype[0], 16, 2, offset);
    offset = message_putvalue(&data, header->ptype[1], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Hardware Length: ", offset);
    offset = message_putvalue(&data, header->hlength, 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Protocol Length: ", offset);
    offset = message_putvalue(&data, header->plength, 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Operation: 0x", offset);
    offset = message_putvalue(&data, header->operation[0], 16, 2, offset);
    offset = message_putvalue(&data, header->operation[1], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);

}

static void print_ipv4(struct channel *channel, unsigned int source, struct ipv4_header *header)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "IPv4:\n", offset);
    offset = message_putstring(&data, "  Protocol: 0x", offset);
    offset = message_putvalue(&data, header->protocol, 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Source Addr (IP): ", offset);
    offset = message_putvalue(&data, header->sip[0], 10, 0, offset);
    offset = message_putstring(&data, ".", offset);
    offset = message_putvalue(&data, header->sip[1], 10, 0, offset);
    offset = message_putstring(&data, ".", offset);
    offset = message_putvalue(&data, header->sip[2], 10, 0, offset);
    offset = message_putstring(&data, ".", offset);
    offset = message_putvalue(&data, header->sip[3], 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Target Addr (IP): ", offset);
    offset = message_putvalue(&data, header->tip[0], 10, 0, offset);
    offset = message_putstring(&data, ".", offset);
    offset = message_putvalue(&data, header->tip[1], 10, 0, offset);
    offset = message_putstring(&data, ".", offset);
    offset = message_putvalue(&data, header->tip[2], 10, 0, offset);
    offset = message_putstring(&data, ".", offset);
    offset = message_putvalue(&data, header->tip[3], 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);

}

static void print_tcp(struct channel *channel, unsigned int source, struct tcp_header *header)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "TCP:\n", offset);
    offset = message_putstring(&data, "  Source Port: ", offset);
    offset = message_putvalue(&data, header->sp[1] | header->sp[0] << 8, 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Target Port: ", offset);
    offset = message_putvalue(&data, header->tp[1] | header->tp[0] << 8, 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Sequence: 0x", offset);
    offset = message_putvalue(&data, header->seq[0], 16, 2, offset);
    offset = message_putvalue(&data, header->seq[1], 16, 2, offset);
    offset = message_putvalue(&data, header->seq[2], 16, 2, offset);
    offset = message_putvalue(&data, header->seq[3], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Ack: 0x", offset);
    offset = message_putvalue(&data, header->ack[0], 16, 2, offset);
    offset = message_putvalue(&data, header->ack[1], 16, 2, offset);
    offset = message_putvalue(&data, header->ack[2], 16, 2, offset);
    offset = message_putvalue(&data, header->ack[3], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Flags: 0x", offset);
    offset = message_putvalue(&data, header->flags[0], 16, 2, offset);
    offset = message_putvalue(&data, header->flags[1], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Window: 0x", offset);
    offset = message_putvalue(&data, header->window[0], 16, 2, offset);
    offset = message_putvalue(&data, header->window[1], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);

}

static void print_udp(struct channel *channel, unsigned int source, struct udp_header *header)
{

    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "UDP:\n", offset);
    offset = message_putstring(&data, "  Source Port: ", offset);
    offset = message_putvalue(&data, header->sp[1] | header->sp[0] << 8, 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Target Port: ", offset);
    offset = message_putvalue(&data, header->tp[1] | header->tp[0] << 8, 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Length: ", offset);
    offset = message_putvalue(&data, header->length[1] | header->length[0] << 8, 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_place(channel, source, EVENT_DATA, offset, &data);

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    file_link(FILE_G0);

    while (channel_pollsource(channel, 0, &header, &data))
    {

        if (header.event == EVENT_DATA)
        {

            struct ethernet_header *eheader = (struct ethernet_header *)data.buffer;

            print_ethernet(channel, source, eheader);

            if (eheader->type[0] == 0x08 && eheader->type[1] == 0x00)
            {

                struct ipv4_header *iheader = (struct ipv4_header *)(eheader + 1);

                print_ipv4(channel, source, iheader);

                if (iheader->protocol == 0x06)
                {

                    struct tcp_header *theader = (struct tcp_header *)(iheader + 1);

                    print_tcp(channel, source, theader);

                }

                else if (iheader->protocol == 0x11)
                {

                    struct udp_header *uheader = (struct udp_header *)(iheader + 1);

                    print_udp(channel, source, uheader);

                }

            }

            if (eheader->type[0] == 0x08 && eheader->type[1] == 0x06)
            {

                struct arp_header *iheader = (struct arp_header *)(eheader + 1);

                print_arp(channel, source, iheader);

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

    channel_setcallback(channel, EVENT_MAIN, onmain);

}

