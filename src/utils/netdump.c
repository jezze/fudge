#include <fudge.h>
#include <net.h>
#include <abi.h>

static void print_icmp(unsigned int source, void *buffer)
{

    struct icmp_header *header = buffer;
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putfmt0(&message, "ICMP:\n");
    message_putfmt1(&message, "  Type: 0x%H2c\n", &header->type[0]);
    message_putfmt1(&message, "  Code: 0x%H2c\n", &header->code[0]);
    channel_sendmessage(&message);

}

static void print_tcp(unsigned int source, void *buffer)
{

    struct tcp_header *header = buffer;
    struct message message;
    unsigned short sp = net_load16(header->sp);
    unsigned short tp = net_load16(header->tp);

    message_init(&message, EVENT_DATA);
    message_putfmt0(&message, "TCP:\n");
    message_putfmt1(&message, "  Source Port: %h\n", &sp);
    message_putfmt1(&message, "  Target Port: %h\n", &tp);
    message_putfmt4(&message, "  Sequence: 0x%H2c%H2c%H2c%H2c\n", &header->seq[0], &header->seq[1], &header->seq[2], &header->seq[3]);
    message_putfmt4(&message, "  Ack: 0x%H2c%H2c%H2c%H2c\n", &header->ack[0], &header->ack[1], &header->ack[2], &header->ack[3]);
    message_putfmt2(&message, "  Flags: 0x%H2c%H2c\n", &header->flags[0], &header->flags[1]);
    message_putfmt2(&message, "  Window: 0x%H2c%H2c\n", &header->window[0], &header->window[1]);
    channel_sendmessage(&message);

}

static void print_udp(unsigned int source, void *buffer)
{

    struct udp_header *header = buffer;
    struct message message;
    unsigned short sp = net_load16(header->sp);
    unsigned short tp = net_load16(header->tp);
    unsigned short length = net_load16(header->length);

    message_init(&message, EVENT_DATA);
    message_putfmt0(&message, "UDP:\n");
    message_putfmt1(&message, "  Source Port: %h\n", &sp);
    message_putfmt1(&message, "  Target Port: %h\n", &tp);
    message_putfmt1(&message, "  Length: %h\n", &length);
    channel_sendmessage(&message);

}

static void print_arp(unsigned int source, void *buffer)
{

    struct arp_header *header = buffer;
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putfmt0(&message, "ARP:\n");
    message_putfmt2(&message, "  Hardware Type: 0x%H2c%H2c\n", &header->htype[0], &header->htype[1]);
    message_putfmt2(&message, "  Protocol Type: 0x%H2c%H2c\n", &header->ptype[0], &header->ptype[1]);
    message_putfmt1(&message, "  Hardware Length: 0x%H2c\n", &header->hlength[0]);
    message_putfmt1(&message, "  Protocol Length: 0x%H2c\n", &header->plength[0]);
    message_putfmt2(&message, "  Operation: 0x%H2c%H2c\n", &header->operation[0], &header->operation[1]);
    channel_sendmessage(&message);

}

static void print_ipv4(unsigned int source, void *buffer)
{

    struct ipv4_header *header = buffer;
    void *payload = (char *)buffer + ipv4_hlen(header);
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putfmt0(&message, "IPv4:\n");
    message_putfmt1(&message, "  Protocol: 0x%H2c\n", &header->protocol[0]);
    message_putfmt4(&message, "  Source Addr (IP): %c.%c.%c.%c\n", &header->sip[0], &header->sip[1], &header->sip[2], &header->sip[3]);
    message_putfmt4(&message, "  Target Addr (IP): %c.%c.%c.%c\n", &header->tip[0], &header->tip[1], &header->tip[2], &header->tip[3]);
    channel_sendmessage(&message);

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
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "IPv6:\n");
    message_putstring(&message, "  Next: 0x");
    message_putvalue(&message, header->next[0], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Hop: 0x");
    message_putvalue(&message, header->hop[0], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Source Addr (IP): ");
    message_putvalue(&message, header->sip[0], 16, 2);
    message_putvalue(&message, header->sip[1], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sip[2], 16, 2);
    message_putvalue(&message, header->sip[3], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sip[4], 16, 2);
    message_putvalue(&message, header->sip[5], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sip[6], 16, 2);
    message_putvalue(&message, header->sip[7], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sip[8], 16, 2);
    message_putvalue(&message, header->sip[9], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sip[10], 16, 2);
    message_putvalue(&message, header->sip[11], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sip[12], 16, 2);
    message_putvalue(&message, header->sip[13], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sip[14], 16, 2);
    message_putvalue(&message, header->sip[15], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Target Addr (IP): ");
    message_putvalue(&message, header->tip[0], 16, 2);
    message_putvalue(&message, header->tip[1], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tip[2], 16, 2);
    message_putvalue(&message, header->tip[3], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tip[4], 16, 2);
    message_putvalue(&message, header->tip[5], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tip[6], 16, 2);
    message_putvalue(&message, header->tip[7], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tip[8], 16, 2);
    message_putvalue(&message, header->tip[9], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tip[10], 16, 2);
    message_putvalue(&message, header->tip[11], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tip[12], 16, 2);
    message_putvalue(&message, header->tip[13], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tip[14], 16, 2);
    message_putvalue(&message, header->tip[15], 16, 2);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void print_ethernet(unsigned int source, void *buffer)
{

    struct ethernet_header *header = buffer;
    void *payload = (char *)buffer + ethernet_hlen(header);
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putfmt0(&message, "Ethernet:\n");
    message_putfmt6(&message, "  Source Addr (MAC): %H2c:%H2c:%H2c:%H2c:%H2c:%H2c\n", &header->sha[0], &header->sha[1], &header->sha[2], &header->sha[3], &header->sha[4], &header->sha[5]);
    message_putfmt6(&message, "  Target Addr (MAC): %H2c:%H2c:%H2c:%H2c:%H2c:%H2c\n", &header->tha[0], &header->tha[1], &header->tha[2], &header->tha[3], &header->tha[4], &header->tha[5]);
    message_putfmt2(&message, "  Type: 0x%H2c%H2c\n", &header->type[0], &header->type[1]);
    channel_sendmessage(&message);

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

    struct message message;

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        channel_error("Could not find ethernet device");

    if (!file_walk(FILE_G0, FILE_L0, "data"))
        channel_error("Could not find ethernet device data");

    file_link(FILE_G0);

    while (channel_readmessage(EVENT_DATA, &message))
        print_ethernet(source, message.data.buffer);

    file_unlink(FILE_G0);
    channel_close();

}

void init(void)
{

    option_add("ethernet", "system:ethernet/if:0");
    channel_bind(EVENT_MAIN, onmain);

}

