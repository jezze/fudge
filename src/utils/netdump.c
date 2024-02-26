#include <fudge.h>
#include <net.h>
#include <abi.h>

static void print_icmp(unsigned int source, void *buffer)
{

    struct icmp_header *header = buffer;

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "ICMP:\n");
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Type: 0x%H2c\n", &header->type[0]);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Code: 0x%H2c\n", &header->code[0]);

}

static void print_tcp(unsigned int source, void *buffer)
{

    struct tcp_header *header = buffer;
    unsigned short sp = net_load16(header->sp);
    unsigned short tp = net_load16(header->tp);

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "TCP:\n");
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Source Port: %h\n", &sp);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Target Port: %h\n", &tp);
    channel_send_fmt4(CHANNEL_DEFAULT, EVENT_DATA, "  Sequence: 0x%H2c%H2c%H2c%H2c\n", &header->seq[0], &header->seq[1], &header->seq[2], &header->seq[3]);
    channel_send_fmt4(CHANNEL_DEFAULT, EVENT_DATA, "  Ack: 0x%H2c%H2c%H2c%H2c\n", &header->ack[0], &header->ack[1], &header->ack[2], &header->ack[3]);
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "  Flags: 0x%H2c%H2c\n", &header->flags[0], &header->flags[1]);
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "  Window: 0x%H2c%H2c\n", &header->window[0], &header->window[1]);

}

static void print_udp(unsigned int source, void *buffer)
{

    struct udp_header *header = buffer;
    unsigned short sp = net_load16(header->sp);
    unsigned short tp = net_load16(header->tp);
    unsigned short length = net_load16(header->length);

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "UDP:\n");
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Source Port: %h\n", &sp);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Target Port: %h\n", &tp);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Length: %h\n", &length);

}

static void print_arp(unsigned int source, void *buffer)
{

    struct arp_header *header = buffer;

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "ARP:\n");
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "  Hardware Type: 0x%H2c%H2c\n", &header->htype[0], &header->htype[1]);
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "  Protocol Type: 0x%H2c%H2c\n", &header->ptype[0], &header->ptype[1]);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Hardware Length: 0x%H2c\n", &header->hlength[0]);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Protocol Length: 0x%H2c\n", &header->plength[0]);
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "  Operation: 0x%H2c%H2c\n", &header->operation[0], &header->operation[1]);

}

static void print_ipv4(unsigned int source, void *buffer)
{

    struct ipv4_header *header = buffer;
    void *payload = (char *)buffer + ipv4_hlen(header);

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "IPv4:\n");
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Protocol: 0x%H2c\n", &header->protocol[0]);
    channel_send_fmt4(CHANNEL_DEFAULT, EVENT_DATA, "  Source Addr (IP): %c.%c.%c.%c\n", &header->sip[0], &header->sip[1], &header->sip[2], &header->sip[3]);
    channel_send_fmt4(CHANNEL_DEFAULT, EVENT_DATA, "  Target Addr (IP): %c.%c.%c.%c\n", &header->tip[0], &header->tip[1], &header->tip[2], &header->tip[3]);

    switch (net_load8(header->protocol))
    {

    case IPV4_PROTOCOL_ICMP:
        print_icmp(source, payload);

        break;

    case IPV4_PROTOCOL_TCP:
        print_tcp(source, payload);

        break;

    case IPV4_PROTOCOL_UDP:
        print_udp(source, payload);

        break;

    }

}

static void print_ipv6(unsigned int source, void *buffer)
{

    struct ipv6_header *header = buffer;

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "IPv6:\n");
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Next: 0x%H2c\n", &header->next[0]);
    channel_send_fmt1(CHANNEL_DEFAULT, EVENT_DATA, "  Hop: 0x%H2c\n", &header->hop[0]);
    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "  Source Addr (IP): ");
    channel_send_fmt8(CHANNEL_DEFAULT, EVENT_DATA, "%H2c%H2c:%H2c%H2c:%H2c%H2c:%H2c%H2c:", &header->sip[0], &header->sip[1], &header->sip[2], &header->sip[3], &header->sip[4], &header->sip[5], &header->sip[6], &header->sip[7]);
    channel_send_fmt8(CHANNEL_DEFAULT, EVENT_DATA, "%H2c%H2c:%H2c%H2c:%H2c%H2c:%H2c%H2c\n", &header->sip[8], &header->sip[9], &header->sip[10], &header->sip[11], &header->sip[12], &header->sip[13], &header->sip[14], &header->sip[15]);
    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "  Target Addr (IP): ");
    channel_send_fmt8(CHANNEL_DEFAULT, EVENT_DATA, "%H2c%H2c:%H2c%H2c:%H2c%H2c:%H2c%H2c:", &header->tip[0], &header->tip[1], &header->tip[2], &header->tip[3], &header->tip[4], &header->tip[5], &header->tip[6], &header->tip[7]);
    channel_send_fmt8(CHANNEL_DEFAULT, EVENT_DATA, "%H2c%H2c:%H2c%H2c:%H2c%H2c:%H2c%H2c\n", &header->tip[8], &header->tip[9], &header->tip[10], &header->tip[11], &header->tip[12], &header->tip[13], &header->tip[14], &header->tip[15]);

}

static void print_ethernet(unsigned int source, void *buffer)
{

    struct ethernet_header *header = buffer;
    void *payload = (char *)buffer + ethernet_hlen(header);

    channel_send_fmt0(CHANNEL_DEFAULT, EVENT_DATA, "Ethernet:\n");
    channel_send_fmt6(CHANNEL_DEFAULT, EVENT_DATA, "  Source Addr (MAC): %H2c:%H2c:%H2c:%H2c:%H2c:%H2c\n", &header->sha[0], &header->sha[1], &header->sha[2], &header->sha[3], &header->sha[4], &header->sha[5]);
    channel_send_fmt6(CHANNEL_DEFAULT, EVENT_DATA, "  Target Addr (MAC): %H2c:%H2c:%H2c:%H2c:%H2c:%H2c\n", &header->tha[0], &header->tha[1], &header->tha[2], &header->tha[3], &header->tha[4], &header->tha[5]);
    channel_send_fmt2(CHANNEL_DEFAULT, EVENT_DATA, "  Type: 0x%H2c%H2c\n", &header->type[0], &header->type[1]);

    switch (net_load16(header->type))
    {

    case ETHERNET_TYPE_ARP:
        print_arp(source, payload);

        break;

    case ETHERNET_TYPE_IPV4:
        print_ipv4(source, payload);

        break;

    case ETHERNET_TYPE_IPV6:
        print_ipv6(source, payload);

        break;

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char data[MESSAGE_SIZE];
    unsigned int count;

    if (!call_walk_absolute(FILE_L0, option_getstring("ethernet")))
        PANIC();

    if (!call_walk_relative(FILE_G0, FILE_L0, "data"))
        PANIC();

    call_link(FILE_G0, 8000);

    while ((count = channel_read_any(EVENT_DATA, data)))
        print_ethernet(source, data);

    call_unlink(FILE_G0);

}

void init(void)
{

    option_add("ethernet", "system:ethernet/if.0");
    channel_bind(EVENT_MAIN, onmain);

}

