#include <fudge.h>
#include <net.h>
#include <abi.h>

static void print_icmp(unsigned int source, void *buffer)
{

    struct icmp_header *header = buffer;
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "ICMP:\n");
    message_putstring(&message, "  Type: 0x");
    message_putvalue(&message, header->type[0], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Code: 0x");
    message_putvalue(&message, header->code[0], 16, 2);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void print_tcp(unsigned int source, void *buffer)
{

    struct tcp_header *header = buffer;
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "TCP:\n");
    message_putstring(&message, "  Source Port: ");
    message_putvalue(&message, net_load16(header->sp), 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Target Port: ");
    message_putvalue(&message, net_load16(header->tp), 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Sequence: 0x");
    message_putvalue(&message, header->seq[0], 16, 2);
    message_putvalue(&message, header->seq[1], 16, 2);
    message_putvalue(&message, header->seq[2], 16, 2);
    message_putvalue(&message, header->seq[3], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Ack: 0x");
    message_putvalue(&message, header->ack[0], 16, 2);
    message_putvalue(&message, header->ack[1], 16, 2);
    message_putvalue(&message, header->ack[2], 16, 2);
    message_putvalue(&message, header->ack[3], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Flags: 0x");
    message_putvalue(&message, header->flags[0], 16, 2);
    message_putvalue(&message, header->flags[1], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Window: 0x");
    message_putvalue(&message, header->window[0], 16, 2);
    message_putvalue(&message, header->window[1], 16, 2);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void print_udp(unsigned int source, void *buffer)
{

    struct udp_header *header = buffer;
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "UDP:\n");
    message_putstring(&message, "  Source Port: ");
    message_putvalue(&message, net_load16(header->sp), 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Target Port: ");
    message_putvalue(&message, net_load16(header->tp), 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Length: ");
    message_putvalue(&message, net_load16(header->length), 10, 0);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void print_arp(unsigned int source, void *buffer)
{

    struct arp_header *header = buffer;
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "ARP:\n");
    message_putstring(&message, "  Hardware Type: 0x");
    message_putvalue(&message, header->htype[0], 16, 2);
    message_putvalue(&message, header->htype[1], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Protocol Type: 0x");
    message_putvalue(&message, header->ptype[0], 16, 2);
    message_putvalue(&message, header->ptype[1], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Hardware Length: ");
    message_putvalue(&message, header->hlength[0], 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Protocol Length: ");
    message_putvalue(&message, header->plength[0], 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Operation: 0x");
    message_putvalue(&message, header->operation[0], 16, 2);
    message_putvalue(&message, header->operation[1], 16, 2);
    message_putstring(&message, "\n");
    channel_sendmessage(&message);

}

static void print_ipv4(unsigned int source, void *buffer)
{

    struct ipv4_header *header = buffer;
    void *payload = (char *)buffer + ipv4_hlen(header);
    struct message message;

    message_init(&message, EVENT_DATA);
    message_putstring(&message, "IPv4:\n");
    message_putstring(&message, "  Protocol: 0x");
    message_putvalue(&message, header->protocol[0], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Source Addr (IP): ");
    message_putvalue(&message, header->sip[0], 10, 0);
    message_putstring(&message, ".");
    message_putvalue(&message, header->sip[1], 10, 0);
    message_putstring(&message, ".");
    message_putvalue(&message, header->sip[2], 10, 0);
    message_putstring(&message, ".");
    message_putvalue(&message, header->sip[3], 10, 0);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Target Addr (IP): ");
    message_putvalue(&message, header->tip[0], 10, 0);
    message_putstring(&message, ".");
    message_putvalue(&message, header->tip[1], 10, 0);
    message_putstring(&message, ".");
    message_putvalue(&message, header->tip[2], 10, 0);
    message_putstring(&message, ".");
    message_putvalue(&message, header->tip[3], 10, 0);
    message_putstring(&message, "\n");
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
    message_putstring(&message, "Ethernet:\n");
    message_putstring(&message, "  Source Addr (MAC): ");
    message_putvalue(&message, header->sha[0], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sha[1], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sha[2], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sha[3], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sha[4], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->sha[5], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Target Addr (MAC): ");
    message_putvalue(&message, header->tha[0], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tha[1], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tha[2], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tha[3], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tha[4], 16, 2);
    message_putstring(&message, ":");
    message_putvalue(&message, header->tha[5], 16, 2);
    message_putstring(&message, "\n");
    message_putstring(&message, "  Type: 0x");
    message_putvalue(&message, header->type[0], 16, 2);
    message_putvalue(&message, header->type[1], 16, 2);
    message_putstring(&message, "\n");
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

    if (!file_walk2(FILE_L0, option_getstring("ethernet")))
        channel_warning("Could not open ethernet");

    if (file_walk(FILE_L1, FILE_L0, "data"))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_link(FILE_L1);

        while ((count = channel_read(buffer, BUFFER_SIZE)))
            print_ethernet(source, buffer);

        file_unlink(FILE_L1);

    }

    else
    {

        channel_error("Could not find data");

    }

    channel_close();

}

void init(void)
{

    option_add("ethernet", "system:ethernet/if:0");
    channel_bind(EVENT_MAIN, onmain);

}

