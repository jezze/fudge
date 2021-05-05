#include <fudge.h>
#include <net.h>
#include <abi.h>

static void print_icmp(struct channel *channel, unsigned int source, void *buffer)
{

    struct icmp_header *header = buffer;
    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "ICMP:\n", offset);
    offset = message_putstring(&data, "  Type: 0x", offset);
    offset = message_putvalue(&data, header->type, 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Code: 0x", offset);
    offset = message_putvalue(&data, header->code, 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_reply(channel, EVENT_DATA, offset, &data);

}

static void print_tcp(struct channel *channel, unsigned int source, void *buffer)
{

    struct tcp_header *header = buffer;
    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "TCP:\n", offset);
    offset = message_putstring(&data, "  Source Port: ", offset);
    offset = message_putvalue(&data, net_load16(header->sp), 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Target Port: ", offset);
    offset = message_putvalue(&data, net_load16(header->tp), 10, 0, offset);
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

    channel_reply(channel, EVENT_DATA, offset, &data);

}

static void print_udp(struct channel *channel, unsigned int source, void *buffer)
{

    struct udp_header *header = buffer;
    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "UDP:\n", offset);
    offset = message_putstring(&data, "  Source Port: ", offset);
    offset = message_putvalue(&data, net_load16(header->sp), 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Target Port: ", offset);
    offset = message_putvalue(&data, net_load16(header->tp), 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Length: ", offset);
    offset = message_putvalue(&data, net_load16(header->length), 10, 0, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_reply(channel, EVENT_DATA, offset, &data);

}

static void print_arp(struct channel *channel, unsigned int source, void *buffer)
{

    struct arp_header *header = buffer;
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

    channel_reply(channel, EVENT_DATA, offset, &data);

}

static void print_ipv4(struct channel *channel, unsigned int source, void *buffer)
{

    struct ipv4_header *header = buffer;
    void *payload = (char *)buffer + ipv4_hlen(header);
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

    channel_reply(channel, EVENT_DATA, offset, &data);

    switch (header->protocol)
    {

    case IPV4_PROTOCOL_ICMP:
        print_icmp(channel, source, payload);

        break;

    case IPV4_PROTOCOL_TCP:
        print_tcp(channel, source, payload);

        break;

    case IPV4_PROTOCOL_UDP:
        print_udp(channel, source, payload);

        break;

    }

}

static void print_ipv6(struct channel *channel, unsigned int source, void *buffer)
{

    struct ipv6_header *header = buffer;
    struct message_data data;
    unsigned int offset = 0;

    offset = message_putstring(&data, "IPv6:\n", offset);
    offset = message_putstring(&data, "  Next: 0x", offset);
    offset = message_putvalue(&data, header->next, 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Hop: 0x", offset);
    offset = message_putvalue(&data, header->hop, 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Source Addr (IP): ", offset);
    offset = message_putvalue(&data, header->sip[0], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[1], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sip[2], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[3], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sip[4], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[5], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sip[6], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[7], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sip[8], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[9], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sip[10], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[11], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sip[12], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[13], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->sip[14], 16, 2, offset);
    offset = message_putvalue(&data, header->sip[15], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);
    offset = message_putstring(&data, "  Target Addr (IP): ", offset);
    offset = message_putvalue(&data, header->tip[0], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[1], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tip[2], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[3], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tip[4], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[5], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tip[6], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[7], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tip[8], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[9], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tip[10], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[11], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tip[12], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[13], 16, 2, offset);
    offset = message_putstring(&data, ":", offset);
    offset = message_putvalue(&data, header->tip[14], 16, 2, offset);
    offset = message_putvalue(&data, header->tip[15], 16, 2, offset);
    offset = message_putstring(&data, "\n", offset);

    channel_reply(channel, EVENT_DATA, offset, &data);

}

static void print_ethernet(struct channel *channel, unsigned int source, void *buffer)
{

    struct ethernet_header *header = buffer;
    void *payload = (char *)buffer + ethernet_hlen(header);
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

    channel_reply(channel, EVENT_DATA, offset, &data);

    switch (net_load16(header->type))
    {

    case ETHERNET_TYPE_ARP:
        print_arp(channel, source, payload);

        break;

    case ETHERNET_TYPE_IPV4:
        print_ipv4(channel, source, payload);

        break;

    case ETHERNET_TYPE_IPV6:
        print_ipv6(channel, source, payload);

        break;

    }

}

static void onmain(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    struct message_header header;
    struct message_data data;

    if (file_walk(FILE_L0, FILE_G0, "data"))
    {

        file_link(FILE_L0);

        while (channel_polldescriptorevent(channel, FILE_L0, EVENT_DATA, &header, &data))
            print_ethernet(channel, source, data.buffer);

        file_unlink(FILE_L0);

    }

    channel_close(channel);

}

static void onoption(struct channel *channel, unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + ascii_lengthz(key);

    if (ascii_match(key, "ethernet"))
        file_walk2(FILE_G0, value);

}

void init(struct channel *channel)
{

    file_walk2(FILE_G0, "system:ethernet/if:0");
    channel_setcallback(channel, EVENT_MAIN, onmain);
    channel_setcallback(channel, EVENT_OPTION, onoption);

}

